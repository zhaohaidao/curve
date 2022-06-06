/*
 *  Copyright (c) 2022 NetEase Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Project: Curve
 * Created Date: 2022-03-16
 * Author: Jingli Chen (Wine93)
 */

#ifndef CURVEFS_SRC_METASERVER_STORAGE_CONVERTER_H_
#define CURVEFS_SRC_METASERVER_STORAGE_CONVERTER_H_


#include <google/protobuf/message.h>
#include <string>
#include <type_traits>

#include "curvefs/src/metaserver/storage/common.h"

namespace curvefs {
namespace metaserver {

class MetaStoreFStream;

namespace storage {

enum KEY_TYPE : unsigned char {
    kTypeInode = 1,
    kTypeS3ChunkInfo = 2,
    kTypeDentry = 3,
    kTypeVolumeExtent = 4,
};

class StorageKey {
 public:
    virtual ~StorageKey() = default;

    virtual std::string SerializeToString() const = 0;
    virtual bool ParseFromString(const std::string& value) = 0;
};

/* rules for key serialization:
 *   Key4Inode                        : kTypeInode:fsId:InodeId
 *   Prefix4AllInode                  : kTypeInode:
 *   Key4S3ChunkInfoList              : kTypeS3ChunkInfo:fsId:inodeId:chunkIndex:firstChunkId:lastChunkId  // NOLINT
 *   Prefix4ChunkIndexS3ChunkInfoList : kTypeS3ChunkInfo:fsId:inodeId:chunkIndex:  // NOLINT
 *   Prefix4InodeS3ChunkInfoList      : kTypeS3ChunkInfo:fsId:inodeId:
 *   Prefix4AllS3ChunkInfoList        : kTypeS3ChunkInfo:
 *   Key4VolumeExtentSlice            : kTypeExtent:fsId:InodeId:SliceOffset
 *   Prefix4InodeVolumeExtent         : kTypeExtent:fsId:InodeId:
 *   Prefix4AllVolumeExtent           : kTypeExtent:
 */

class Key4Inode : public StorageKey {
 public:
    Key4Inode();

    Key4Inode(uint32_t fsId, uint64_t inodeId);

    explicit Key4Inode(const Inode& inode);

    bool operator==(const Key4Inode& rhs);

    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 public:
    static const KEY_TYPE keyType_ = kTypeInode;

    uint32_t fsId;
    uint64_t inodeId;
};

class Prefix4AllInode : public StorageKey {
 public:
    Prefix4AllInode() = default;

    std::string SerializeToString() const override;

     bool ParseFromString(const std::string& value) override;

 public:
    static const KEY_TYPE keyType_ = kTypeInode;
};

class Key4S3ChunkInfoList : public StorageKey {
 public:
    Key4S3ChunkInfoList();

    Key4S3ChunkInfoList(uint32_t fsId,
                        uint64_t inodeId,
                        uint64_t chunkIndex,
                        uint64_t firstChunkId,
                        uint64_t lastChunkId,
                        uint64_t size);

    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 public:
    static const size_t kMaxUint64Length_;
    static const KEY_TYPE keyType_ = kTypeS3ChunkInfo;

     uint32_t fsId;
     uint64_t inodeId;
     uint64_t chunkIndex;
     uint64_t firstChunkId;
     uint64_t lastChunkId;
     uint64_t size;
};

class Prefix4ChunkIndexS3ChunkInfoList : public StorageKey {
 public:
    Prefix4ChunkIndexS3ChunkInfoList();

    Prefix4ChunkIndexS3ChunkInfoList(uint32_t fsId,
                                     uint64_t inodeId,
                                     uint64_t chunkIndex);

    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 public:
    static const KEY_TYPE keyType_ = kTypeS3ChunkInfo;

    uint32_t fsId;
    uint64_t inodeId;
    uint64_t chunkIndex;
};

class Prefix4InodeS3ChunkInfoList : public StorageKey {
 public:
    Prefix4InodeS3ChunkInfoList();

    Prefix4InodeS3ChunkInfoList(uint32_t fsId,
                                uint64_t inodeId);

    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 public:
    static const KEY_TYPE keyType_ = kTypeS3ChunkInfo;

    uint32_t fsId;
    uint64_t inodeId;
};

class Prefix4AllS3ChunkInfoList : public StorageKey {
 public:
    Prefix4AllS3ChunkInfoList() = default;

    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 public:
    static const KEY_TYPE keyType_ = kTypeS3ChunkInfo;
};

class Key4VolumeExtentSlice : public StorageKey {
 public:
    Key4VolumeExtentSlice() = default;

    Key4VolumeExtentSlice(uint32_t fsId, uint64_t inodeId, uint64_t offset);

    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 private:
    friend class curvefs::metaserver::MetaStoreFStream;

    uint32_t fsId_;
    uint64_t inodeId_;
    uint64_t offset_;

    static constexpr KEY_TYPE keyType_ = kTypeVolumeExtent;
};

class Prefix4InodeVolumeExtent : public StorageKey {
 public:
    Prefix4InodeVolumeExtent(uint32_t fsId, uint64_t inodeId);

    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 private:
    uint32_t fsId_;
    uint64_t inodeId_;

    static constexpr KEY_TYPE keyType_ = kTypeVolumeExtent;
};

class Prefix4AllVolumeExtent : public StorageKey {
 public:
    std::string SerializeToString() const override;

    bool ParseFromString(const std::string& value) override;

 private:
    static constexpr KEY_TYPE keyType_ = kTypeVolumeExtent;
};

// converter
class Converter {
 public:
    Converter() = default;

    // for key
    std::string SerializeToString(const StorageKey& key);

    // for value
    bool SerializeToString(const google::protobuf::Message& entry,
                           std::string* value);

    // for key&value
    template <typename Entry,
              typename = typename std::enable_if<
                  std::is_base_of<google::protobuf::Message, Entry>::value ||
                  std::is_base_of<StorageKey, Entry>::value>::type>
    bool ParseFromString(const std::string& value, Entry* entry) {
        return entry->ParseFromString(value);
    }
};

}  // namespace storage
}  // namespace metaserver
}  // namespace curvefs

#endif  // CURVEFS_SRC_METASERVER_STORAGE_CONVERTER_H_