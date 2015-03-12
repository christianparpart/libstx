/*
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _FNORD_FTS_HUNSPELL_H
#define _FNORD_FTS_HUNSPELL_H
#include "fnord-base/stdtypes.h"
#include "hunspell/hunspell.h"

namespace fnord {
namespace fts {

class Hunspell {
public:

  Hunspell(const String& aff_file, const String& dict_file);

protected:
  Hunhandle* handle_;
};

} // namespace fts
} // namespace fnord

#endif
