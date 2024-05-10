/*
 * Copyright 2016-2023 ClickHouse, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * This file may have been modified by Bytedance Ltd. and/or its affiliates (“ Bytedance's Modifications”).
 * All Bytedance's Modifications are Copyright (2023) Bytedance Ltd. and/or its affiliates.
 */

#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <Disks/IDisk.h>

namespace DB
{
class DiskMemory;
class ReadBufferFromFileBase;
class WriteBufferFromFileBase;


/** Implementation of Disk intended only for testing purposes.
  * All filesystem objects are stored in memory and lost on server restart.
  *
  * NOTE Work in progress. Currently the interface is not viable enough to support MergeTree or even StripeLog tables.
  * Please delete this interface if it will not be finished after 2020-06-18.
  */
class DiskMemory : public IDisk
{
public:
    DiskMemory(const String & name_) : name(name_), disk_path("memory://" + name_ + '/') {}

    UInt64 getID() const override;

    const String & getName() const override { return name; }

    const String & getPath() const override { return disk_path; }

    ReservationPtr reserve(UInt64 bytes) override;

    DiskStats getTotalSpace([[maybe_unused]]bool with_keep_free = false) const override;

    DiskStats getAvailableSpace() const override;

    DiskStats getUnreservedSpace() const override;

    bool exists(const String & path) const override;

    bool isFile(const String & path) const override;

    bool isDirectory(const String & path) const override;

    size_t getFileSize(const String & path) const override;

    void createDirectory(const String & path) override;

    void createDirectories(const String & path) override;

    void clearDirectory(const String & path) override;

    void moveDirectory(const String & from_path, const String & to_path) override;

    DiskDirectoryIteratorPtr iterateDirectory(const String & path) override;

    void createFile(const String & path) override;

    void moveFile(const String & from_path, const String & to_path) override;

    void replaceFile(const String & from_path, const String & to_path) override;

    void listFiles(const String & path, std::vector<String> & file_names) override;

    std::unique_ptr<ReadBufferFromFileBase> readFile(
        const String & path,
        const ReadSettings& settings) const override;

    std::unique_ptr<WriteBufferFromFileBase> writeFile(
        const String & path,
        const WriteSettings& settings) override;

    void removeFile(const String & path) override;
    void removeFileIfExists(const String & path) override;
    void removeDirectory(const String & path) override;
    void removeRecursive(const String & path) override;

    void setLastModified(const String &, const Poco::Timestamp &) override {}

    Poco::Timestamp getLastModified(const String &) override { return Poco::Timestamp(); }

    void setReadOnly(const String & path) override;

    void createHardLink(const String & src_path, const String & dst_path) override;

    void truncateFile(const String & path, size_t size) override;

    DiskType::Type getType() const override { return DiskType::Type::RAM; }

private:
    void createDirectoriesImpl(const String & path);
    void replaceFileImpl(const String & from_path, const String & to_path);

private:
    friend class WriteIndirectBuffer;

    enum class FileType
    {
        File,
        Directory
    };

    struct FileData
    {
        FileType type;
        String data;

        FileData(FileType type_, String data_) : type(type_), data(std::move(data_)) {}
        explicit FileData(FileType type_) : type(type_), data("") {}
    };
    using Files = std::unordered_map<String, FileData>; /// file path -> file data

    const String name;
    const String disk_path;
    Files files;
    mutable std::mutex mutex;
};

}
