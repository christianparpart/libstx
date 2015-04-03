/**
 * This file is part of the "libfnord" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <fnord-cstable/ColumnWriter.h>

namespace fnord {
namespace cstable {

ColumnWriter::ColumnWriter(
    size_t r_max,
    size_t d_max) :
    r_max_(r_max),
    d_max_(d_max),
    rlvl_writer_(r_max),
    dlvl_writer_(d_max) {}

size_t ColumnWriter::maxRepetitionLevel() const {
  return r_max_;
}

size_t ColumnWriter::maxDefinitionLevel() const {
  return d_max_;
}

void ColumnWriter::write(void* buf, size_t buf_len) {
  util::BinaryMessageWriter writer(buf, buf_len);
  writer.appendUInt64(rlvl_writer_.size());
  writer.appendUInt64(dlvl_writer_.size());
  writer.appendUInt64(size());
  writer.append(rlvl_writer_.data(), rlvl_writer_.size());
  writer.append(dlvl_writer_.data(), dlvl_writer_.size());
  write(&writer);
}

size_t ColumnWriter::bodySize() const {
  return 24 + rlvl_writer_.size() + dlvl_writer_.size() + size();
}



} // namespace cstable
} // namespace fnord
