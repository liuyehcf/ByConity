#pragma once

#include <Core/Names.h>
#include <Core/NamesAndAliases.h>
#include <Core/NamesAndTypes.h>
#include <Columns/IColumn.h>
#include <Interpreters/Context.h>

namespace DB
{
/** Allows to log information about unique table execution:
  * - info about errors of query execution.
  */

/// A struct which will be inserted as row into cnch_unique_table_log table
struct UniqueTableLogElement
{
    enum Type
    {
        EMPTY = 0,
        ERROR = 1,
    };

    String database;
    String table;

    Type type;
    time_t event_time{};
    UInt64 txn_id = 0;
    String dedup_task_info;
    String event_info;

    /// some reserved fields(duration_ms), currently not used
    UInt64 duration_ms = 0;
    UInt64 metric = 0;

    UInt8 has_error = 0;
    String event_msg;

    static std::string name() { return "UniqueTableLog"; }
    static NamesAndTypesList getNamesAndTypes();
    static NamesAndAliases getNamesAndAliases() { return {}; }
    void appendToBlock(MutableColumns & columns) const;

};

namespace UniqueTable
{
    UniqueTableLogElement createUniqueTableLog(UniqueTableLogElement::Type type, const StorageID & storage_id, bool has_error = true);

    String formatUniqueKey(const String & unique_index_str_, const StorageMetadataPtr & metadata_snapshot);
}
}
