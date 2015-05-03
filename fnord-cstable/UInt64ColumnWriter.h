/**
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _FNORD_CSTABLE_INT64COLUMNWRITER_H
#define _FNORD_CSTABLE_INT64COLUMNWRITER_H
#include <fnord-base/stdtypes.h>
#include <fnord-base/util/binarymessagewriter.h>
#include <fnord-base/util/BitPackEncoder.h>
#include <fnord-cstable/BinaryFormat.h>
#include <fnord-cstable/ColumnWriter.h>

namespace fnord {
namespace cstable {

class UInt64ColumnWriter : public ColumnWriter {
public:

  UInt64ColumnWriter(
      uint64_t r_max,
      uint64_t d_max);

  void addDatum(
      uint64_t rep_level,
      uint64_t def_level,
      const void* data,
      size_t size) override;

  void addDatum(uint64_t rep_level, uint64_t def_level, uint64_t value);

  ColumnType type() const override {
    return ColumnType::UINT64_PLAIN;
  }

protected:
  size_t size() const override;
  void write(util::BinaryMessageWriter* writer) override;

  uint64_t max_value_;
  util::BinaryMessageWriter data_writer_;
};

} // namespace cstable
} // namespace fnord

#endif