/**
 * This file is part of the "libstx" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * libstx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _STX_IO_FILEREPOSITORY_H_
#define _STX_IO_FILEREPOSITORY_H_
#include <functional>
#include <string>
#include <vector>
#include "stx/random.h"

namespace stx {

class FileRepository {
public:
  struct FileRef {
    std::string logical_filename;
    std::string absolute_path;
  };

  FileRepository(const std::string& basedir);

  /**
   * Create and return a new file
   */
  FileRef createFile() const;

  void listFiles(
      std::function<bool(const std::string& filename)> callback) const;

  void deleteAllFiles();

protected:
  mutable Random rnd_;
  std::string basedir_;
};

}
#endif
