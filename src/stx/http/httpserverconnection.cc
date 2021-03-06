/**
 * This file is part of the "libstx" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * libstx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "stx/exception.h"
#include "stx/inspect.h"
#include "stx/logging.h"
#include "stx/http/httpserverconnection.h"
#include "stx/http/httpgenerator.h"

namespace stx {

template <>
std::string inspect(const http::HTTPServerConnection& conn) {
  return StringUtil::format("<HTTPServerConnection $0>", inspect(&conn));
}

namespace http {

void HTTPServerConnection::start(
    HTTPHandlerFactory* handler_factory,
    ScopedPtr<net::TCPConnection> conn,
    TaskScheduler* scheduler,
    HTTPServerStats* stats) {
  auto http_conn = new HTTPServerConnection(
      handler_factory,
      std::move(conn),
      scheduler,
      stats);

  // N.B. we don't leak the connection here. it is ref counted and will
  // free itself
  http_conn->incRef();
  http_conn->nextRequest();
}

HTTPServerConnection::HTTPServerConnection(
    HTTPHandlerFactory* handler_factory,
    ScopedPtr<net::TCPConnection> conn,
    TaskScheduler* scheduler,
    HTTPServerStats* stats) :
    handler_factory_(handler_factory),
    conn_(std::move(conn)),
    scheduler_(scheduler),
    parser_(HTTPParser::PARSE_HTTP_REQUEST),
    on_write_completed_cb_(nullptr),
    closed_(false),
    stats_(stats) {
  logTrace("http.server", "New HTTP connection: $0", inspect(*this));
  stats_->total_connections.incr(1);
  stats_->current_connections.incr(1);

  conn_->setNonblocking(true);
  read_buf_.reserve(kMinBufferSize);

  parser_.onMethod([this] (HTTPMessage::kHTTPMethod method) {
    cur_request_->setMethod(method);
  });

  parser_.onURI([this] (const char* data, size_t size) {
    cur_request_->setURI(std::string(data, size));
  });

  parser_.onVersion([this] (const char* data, size_t size) {
    cur_request_->setVersion(std::string(data, size));
  });

  parser_.onHeader([this] (
      const char* key,
      size_t key_size,
      const char* val,
      size_t val_size) {
    cur_request_->addHeader(
        std::string(key, key_size),
        std::string(val, val_size));
  });

  parser_.onHeadersComplete(
      std::bind(&HTTPServerConnection::dispatchRequest, this));
}

HTTPServerConnection::~HTTPServerConnection() {
  stats_->current_connections.decr(1);
}

void HTTPServerConnection::read() {
  std::unique_lock<std::recursive_mutex> lk(mutex_);

  size_t len;
  try {
    len = conn_->read(read_buf_.data(), read_buf_.allocSize());
    stats_->received_bytes.incr(len);
  } catch (Exception& e) {
    if (e.ofType(kWouldBlockError)) {
      return awaitRead();
    }

    lk.unlock();
    logDebug("http.server", e, "read() failed, closing...");

    close();
    return;
  }

  try {
    if (len == 0) {
      parser_.eof();
      lk.unlock();
      close();
      return;
    } else {
      parser_.parse((char *) read_buf_.data(), len);
    }
  } catch (Exception& e) {
    logDebug("http.server", e, "HTTP parse error, closing...");
    lk.unlock();
    close();
    return;
  }

  if (parser_.state() != HTTPParser::S_DONE) {
    awaitRead();
  }
}

void HTTPServerConnection::write() {
  std::unique_lock<std::recursive_mutex> lk(mutex_);

  auto data = ((char *) write_buf_.data()) + write_buf_.mark();
  auto size = write_buf_.size() - write_buf_.mark();

  size_t len;
  try {
    len = conn_->write(data, size);
    write_buf_.setMark(write_buf_.mark() + len);
    stats_->sent_bytes.incr(len);
  } catch (Exception& e) {
    if (e.ofType(kWouldBlockError)) {
      return awaitWrite();
    }

    lk.unlock();
    logDebug("http.server", e, "write() failed, closing...");
    close();
    return;
  }

  if (write_buf_.mark() < write_buf_.size()) {
    awaitWrite();
  } else {
    write_buf_.clear();
    lk.unlock();
    if (on_write_completed_cb_) {
      on_write_completed_cb_();
    }
  }
}

// precondition: must hold mutex
void HTTPServerConnection::awaitRead() {
  if (closed_) {
    RAISE(kIllegalStateError, "read() on closed HTTP connection");
  }

  scheduler_->runOnReadable(
      std::bind(&HTTPServerConnection::read, this),
      *conn_);
}

// precondition: must hold mutex
void HTTPServerConnection::awaitWrite() {
  if (closed_) {
    RAISE(kIllegalStateError, "write() on closed HTTP connection");
  }

  scheduler_->runOnWritable(
      std::bind(&HTTPServerConnection::write, this),
      *conn_);
}

void HTTPServerConnection::nextRequest() {
  parser_.reset();
  cur_request_.reset(new HTTPRequest());
  cur_handler_.reset(nullptr);
  on_write_completed_cb_ = nullptr;
  body_buf_.clear();

  parser_.onBodyChunk([this] (const char* data, size_t size) {
    std::unique_lock<std::recursive_mutex> lk(mutex_);
    body_buf_.append(data, size);
  });

  awaitRead();
}

void HTTPServerConnection::dispatchRequest() {
  stats_->total_requests.incr(1);
  stats_->current_requests.incr(1);

  incRef();
  cur_handler_= handler_factory_->getHandler(this, cur_request_.get());
  cur_handler_->handleHTTPRequest();
}

void HTTPServerConnection::readRequestBody(
    Function<void (const void*, size_t, bool)> callback) {
  std::unique_lock<std::recursive_mutex> lk(mutex_);

  switch (parser_.state()) {
    case HTTPParser::S_REQ_METHOD:
    case HTTPParser::S_REQ_URI:
    case HTTPParser::S_REQ_VERSION:
    case HTTPParser::S_RES_VERSION:
    case HTTPParser::S_RES_STATUS_CODE:
    case HTTPParser::S_RES_STATUS_NAME:
    case HTTPParser::S_HEADER:
      RAISE(kIllegalStateError, "can't read body before headers are parsed");
    case HTTPParser::S_BODY:
    case HTTPParser::S_DONE:
      break;
  }

  auto read_body_chunk_fn = [this, callback] (const char* data, size_t size) {
    std::unique_lock<std::recursive_mutex> lk(mutex_);
    body_buf_.append(data, size);
    auto last_chunk = parser_.state() == HTTPParser::S_DONE;

    if (last_chunk || body_buf_.size() > 0) {
      BufferRef chunk(new Buffer(body_buf_));

      scheduler_->runAsync([callback, chunk, last_chunk] {
        callback(
            (const char*) chunk->data(),
            chunk->size(),
            last_chunk);
      });

      body_buf_.clear();
    }
  };

  parser_.onBodyChunk(read_body_chunk_fn);
  lk.unlock();
  read_body_chunk_fn(nullptr, 0);
}

void HTTPServerConnection::discardRequestBody(Function<void ()> callback) {
  readRequestBody([callback] (const void* data, size_t size, bool last) {
    if (last) {
      callback();
    }
  });
}

void HTTPServerConnection::writeResponse(
    const HTTPResponse& resp,
    Function<void()> ready_callback) {
  std::lock_guard<std::recursive_mutex> lk(mutex_);

  if (parser_.state() != HTTPParser::S_DONE) {
    RAISE(kIllegalStateError, "can't write response before request is read");
  }

  BufferOutputStream os(&write_buf_);
  HTTPGenerator::generate(resp, &os);
  on_write_completed_cb_ = ready_callback;
  awaitWrite();
}

void HTTPServerConnection::writeResponseBody(
    const void* data,
    size_t size,
    Function<void()> ready_callback) {
  std::lock_guard<std::recursive_mutex> lk(mutex_);

  if (parser_.state() != HTTPParser::S_DONE) {
    RAISE(kIllegalStateError, "can't write response before request is read");
  }

  write_buf_.append(data, size);
  on_write_completed_cb_ = ready_callback;
  awaitWrite();
}

void HTTPServerConnection::finishResponse() {
  stats_->current_requests.decr(1);

  if (decRef()) {
    return;
  }

  if (false && cur_request_->keepalive()) {
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    nextRequest();
  } else {
    close();
  }
}

// precondition: must not hold mutex
void HTTPServerConnection::close() {
  std::unique_lock<std::recursive_mutex> lk(mutex_);

  logTrace("http.server", "HTTP connection close: $0", inspect(*this));

  if (closed_) {
    RAISE(kIllegalStateError, "HTTP connection is already closed");
  }

  closed_ = true;
  scheduler_->cancelFD(conn_->fd());
  conn_->close();
  lk.unlock();

  decRef();
}

bool HTTPServerConnection::isClosed() const {
  std::unique_lock<std::recursive_mutex> lk(mutex_);
  return closed_;
}

} // namespace http
} // namespace stx

