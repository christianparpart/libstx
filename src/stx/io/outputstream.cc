/**
 * This file is part of the "libstx" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * libstx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <cstdarg>
#include <fcntl.h>
#include <memory>
#include <stdarg.h>
#include <string>
#include <unistd.h>
#include "stx/buffer.h"
#include "stx/exception.h"
#include "stx/io/outputstream.h"
#include "stx/ieee754.h"

namespace stx {

std::unique_ptr<OutputStream> OutputStream::getStdout() {
  auto stdout_stream = new FileOutputStream(1, false);
  return std::unique_ptr<OutputStream>(stdout_stream);
}

std::unique_ptr<OutputStream> OutputStream::getStderr() {
  auto stderr_stream = new FileOutputStream(2, false);
  return std::unique_ptr<OutputStream>(stderr_stream);
}

size_t OutputStream::write(const std::string& data) {
  return write(data.c_str(), data.size());
}

size_t OutputStream::write(const Buffer& buf) {
  return write((const char*) buf.data(), buf.size());
}

// FIXPAUL: variable size buffer
size_t OutputStream::printf(const char* format, ...) {
  char buf[8192];

  va_list args;
  va_start(args, format);
  int pos = vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  if (pos < 0) {
    RAISE_ERRNO(kIOError, "vsnprintf() failed");
  }

  if (pos < sizeof(buf)) {
    write(buf, pos);
  } else {
    RAISE_ERRNO(kBufferOverflowError, "vsnprintf() failed: value too large");
  }

  return pos;
}

void OutputStream::appendUInt8(uint8_t value) {
  write((char*) &value, sizeof(value));
}

void OutputStream::appendUInt16(uint16_t value) {
  write((char*) &value, sizeof(value));
}

void OutputStream::appendUInt32(uint32_t value) {
  write((char*) &value, sizeof(value));
}

void OutputStream::appendUInt64(uint64_t value) {
  write((char*) &value, sizeof(value));
}

void OutputStream::appendDouble(double value) {
  auto bytes = IEEE754::toBytes(value);
  write((char*) &bytes, sizeof(bytes));
}

void OutputStream::appendString(const std::string& string) {
  write(string.data(), string.size());
}

void OutputStream::appendLenencString(const std::string& string) {
  appendLenencString(string.data(), string.size());
}

void OutputStream::appendLenencString(const void* data, size_t size) {
  appendVarUInt(size);
  write((char*) data, size);
}

void OutputStream::appendVarUInt(uint64_t value) {
  unsigned char buf[10];
  size_t bytes = 0;
  do {
    buf[bytes] = value & 0x7fU;
    if (value >>= 7) buf[bytes] |= 0x80U;
    ++bytes;
  } while (value);

  write((char*) buf, bytes);
}

} // namespace stx
