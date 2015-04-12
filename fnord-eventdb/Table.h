/**
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _FNORD_EVENTDB_TABLE_H
#define _FNORD_EVENTDB_TABLE_H
#include <fnord-base/stdtypes.h>
#include <fnord-base/autoref.h>
#include <fnord-msg/MessageSchema.h>
#include <fnord-msg/MessageObject.h>

namespace fnord {
namespace eventdb {

class Table : public RefCounted {
public:

  Table(
      const String& table_name,
      const msg::MessageSchema& schema);

  void addRecords(const Buffer& records);
  void addRecord(const msg::MessageObject& record);

  const String& name() const;

protected:
   String name_;
   msg::MessageSchema schema_;
};

} // namespace eventdb
} // namespace fnord

#endif
