/**
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _FNORD_DOCTABLE_TABLESEGMENTMERGE_H
#define _FNORD_DOCTABLE_TABLESEGMENTMERGE_H
#include <fnord-base/stdtypes.h>
#include <fnord-base/autoref.h>
#include <fnord-base/random.h>
#include <fnord-base/io/FileLock.h>
#include <fnord-base/thread/taskscheduler.h>
#include <fnord-msg/MessageSchema.h>
#include <fnord-msg/MessageObject.h>
#include <fnord-logtable/ArtifactIndex.h>
#include <fnord-doctable/TableArena.h>
#include <fnord-doctable/TableSnapshot.h>
#include "fnord-sstable/sstablereader.h"
#include "fnord-sstable/sstablewriter.h"
#include "fnord-sstable/SSTableColumnSchema.h"
#include "fnord-sstable/SSTableColumnReader.h"
#include "fnord-sstable/SSTableColumnWriter.h"

namespace fnord {
namespace doctable {

class TableSegmentMerge {
public:

  TableSegmentMerge(
      const String& db_path,
      const String& table_name,
      msg::MessageSchema* schema,
      const Vector<TableSegmentRef> input_chunks,
      TableSegmentRef* output_chunk,
      RefPtr<TableSegmentWriter> writer);

  void merge();

protected:

  void readTable(const String& filename);

  String db_path_;
  String table_name_;
  msg::MessageSchema* schema_;
  RefPtr<TableSegmentWriter> writer_;
  Vector<TableSegmentRef> input_chunks_;
};

class TableMergePolicy {
public:

  TableMergePolicy();

  bool findNextMerge(
      RefPtr<TableSnapshot> snapshot,
      const String& db_path,
      const String& table_name,
      const String& replica_id,
      Vector<TableSegmentRef>* input_chunks,
      TableSegmentRef* output_chunk);

protected:

  bool tryFoldIntoMerge(
      const String& db_path,
      const String& table_name,
      const String& replica_id,
      size_t min_merged_size,
      size_t max_merged_size,
      const Vector<TableSegmentRef>& chunks,
      size_t idx,
      Vector<TableSegmentRef>* input_chunks,
      TableSegmentRef* output_chunk);

  Vector<Pair<uint64_t, uint64_t>> steps_;
  Random rnd_;
};

class TableWriter : public RefCounted {
public:
  typedef Function<RefPtr<TableSegmentSummaryBuilder> ()> SummaryFactoryFn;

  static RefPtr<TableWriter> open(
      const String& table_name,
      const String& replica_id,
      const String& db_path,
      const msg::MessageSchema& schema,
      TaskScheduler* scheduler);

  void addSummary(SummaryFactoryFn summary);
  void addRecords(const Buffer& records);
  void addRecord(const msg::MessageObject& record);

  RefPtr<TableSnapshot> getSnapshot();
  const String& name() const;

  size_t commit();
  void merge();
  void gc(size_t keep_generations = 2, size_t max_generations = 10);

  void replicateFrom(const TableGeneration& other_table);

  size_t arenaSize() const;
  ArtifactIndex* artifactIndex();

  void runConsistencyCheck(bool check_checksums = false, bool repair = false);

protected:

  TableWriter(
      const String& table_name,
      const String& replica_id,
      const String& db_path,
      const msg::MessageSchema& schema,
      uint64_t head_sequence,
      RefPtr<TableGeneration> snapshot,
      TaskScheduler* scheduler);

  void gcArenasWithLock();
  size_t commitWithLock();
  void writeTable(RefPtr<TableArena> arena);
  void addChunk(const TableSegmentRef* chunk, ArtifactStatus status);
  void writeSnapshot();
  bool merge(size_t min_chunk_size, size_t max_chunk_size);

  String name_;
  String replica_id_;
  String db_path_;
  msg::MessageSchema schema_;
  TaskScheduler* scheduler_;
  ArtifactIndex artifacts_;
  mutable std::mutex mutex_;
  std::mutex merge_mutex_;
  uint64_t seq_;
  List<RefPtr<TableArena>> arenas_;
  Random rnd_;
  RefPtr<TableGeneration> head_;
  TableMergePolicy merge_policy_;
  FileLock lock_;
  List<SummaryFactoryFn> summaries_;
  Duration gc_delay_;
};

} // namespace doctable
} // namespace fnord

#endif