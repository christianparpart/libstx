/**
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "fnord-fts/SynonymDictionary.h"

namespace fnord {
namespace fts {

Option<String> SynonymDictionary::lookup(Language lang, const String& term) {
  auto iter = synonyms_.find(synonymKey(lang, term));
  if (iter == synonyms_.end()) {
    return None<String>();
  } else {
    return Some(iter->second);
  }
}

void SynonymDictionary::addSynonym(
    Language lang,
    const String& term,
    const String& canonical_term) {
  synonyms_[synonymKey(lang, term)] = canonical_term;
}

void SynonymDictionary::loadSynonmFile(const String& filename) {
}

} // namespace fts
} // namespace fnord
