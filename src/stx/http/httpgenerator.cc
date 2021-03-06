/**
 * This file is part of the "libstx" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * libstx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <stx/exception.h>
#include <stx/stringutil.h>
#include <stx/http/httpgenerator.h>

using stx::OutputStream;

namespace stx {
namespace http {

void HTTPGenerator::generate(const HTTPRequest& req, OutputStream* os) {
  os->write(
      StringUtil::format(
          "$0 $1 $2\r\n",
          req.method(),
          req.uri(),
          req.version()));

  for (const auto& header : req.headers()) {
    os->write(StringUtil::format("$0: $1\r\n", header.first, header.second));
  }

  if (req.body().size() > 0 && !req.hasHeader("Content-Length")) {
    os->write(StringUtil::format("Content-Length: $0\r\n", req.body().size()));
  }

  os->write("\r\n");

  const auto& body = req.body();
  if (body.size() > 0) {
    os->write(body);
  }
}

void HTTPGenerator::generate(const HTTPResponse& res, OutputStream* os) {
  if (res.version().length() < 4) {
    RAISEF(kRuntimeError, "invalid http version: $0", res.version());
  }

  os->write(
      StringUtil::format(
          "$0 $1 $2\r\n",
          res.version(),
          res.statusCode(),
          res.statusName()));

  for (const auto& header : res.headers()) {
    os->write(StringUtil::format("$0: $1\r\n", header.first, header.second));
  }

  os->write("\r\n");

  const auto& body = res.body();
  if (body.size() > 0) {
    os->write(body);
  }
}

}
}
