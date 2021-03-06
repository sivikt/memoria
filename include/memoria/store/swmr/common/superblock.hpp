
// Copyright 2020 Victor Smirnov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#pragma once

#include <memoria/profiles/common/common.hpp>
#include <memoria/core/tools/result.hpp>
#include <memoria/core/tools/bitmap.hpp>
#include <memoria/core/strings/format.hpp>
#include <memoria/core/packed/tools/packed_allocator.hpp>

namespace memoria {

template <typename Profile>
class SWMRSuperblock {
    using BlockID    = ProfileBlockID<Profile>;
    using CommitUUID = ProfileSnapshotID<Profile>;
    using CommitID   = int64_t;
    using SequenceID = uint64_t;

    char magic_buffer_[256];
    char reserved_[4 * 8];
    SequenceID sequence_id_;
    CommitID commit_id_;
    uint64_t file_size_;
    uint64_t superblock_file_pos_;
    uint64_t superblock_size_;

    BlockID history_root_id_;
    BlockID directory_root_id_;
    BlockID counters_root_id_;
    BlockID allocator_root_id_;

    CommitUUID commit_uuid_;

    PackedAllocator allocator_;

public:
    SWMRSuperblock() noexcept {}

    BlockID& history_root_id() noexcept   {return history_root_id_;}
    BlockID& directory_root_id() noexcept {return directory_root_id_;}
    BlockID& counters_root_id() noexcept  {return counters_root_id_;}
    BlockID& allocator_root_id() noexcept {return allocator_root_id_;}

    const BlockID& history_root_id() const noexcept   {return history_root_id_;}
    const BlockID& directory_root_id() const noexcept {return directory_root_id_;}
    const BlockID& counters_root_id() const noexcept  {return counters_root_id_;}
    const BlockID& allocator_root_id() const noexcept {return allocator_root_id_;}

    const char* magic_buffer() const noexcept {return magic_buffer_;}
    char* magic_buffer() noexcept {return magic_buffer_;}

    const SequenceID& sequence_id() const noexcept {return sequence_id_;}
    SequenceID& sequence_id() noexcept {return sequence_id_;}

    const CommitID& commit_id() const noexcept {return commit_id_;}
    CommitID& commit_id() noexcept {return commit_id_;}

    uint64_t file_size() const noexcept {return file_size_;}
    uint64_t& file_size() noexcept {return file_size_;}

    uint64_t superblock_size() const noexcept {return superblock_size_;}
    uint64_t& superblock_size() noexcept {return superblock_size_;}

    uint64_t superblock_file_pos() const noexcept {return superblock_file_pos_;}
    uint64_t& superblock_file_pos() noexcept {return superblock_file_pos_;}

    const CommitUUID& commit_uuid() const noexcept {return commit_uuid_;}
    CommitUUID& commit_uuid() noexcept {return commit_uuid_;}

    VoidResult init(uint64_t superblock_file_pos, uint64_t file_size, int64_t commit_id, size_t superblock_size)
    {
        std::memset(magic_buffer_, 0, sizeof(magic_buffer_));
        std::memset(reserved_, 0, sizeof(reserved_));

        sequence_id_ = 1;
        commit_id_   = commit_id;

        superblock_file_pos_ = superblock_file_pos;
        file_size_           = file_size;
        superblock_size_     = superblock_size;

        history_root_id_   = BlockID{};
        directory_root_id_ = BlockID{};
        counters_root_id_  = BlockID{};
        allocator_root_id_ = BlockID{};

        commit_uuid_       = ProfileTraits<Profile>::make_random_snapshot_id();

        return allocator_.init(allocator_block_size(superblock_size), 1);
    }

    VoidResult init_from(const SWMRSuperblock& other, uint64_t superblock_file_pos, int64_t commit_id) noexcept
    {
        history_root_id_   = other.history_root_id_;
        directory_root_id_ = other.directory_root_id_;
        counters_root_id_  = other.counters_root_id_;
        allocator_root_id_ = other.allocator_root_id_;

        sequence_id_ = other.sequence_id_ + 1;
        commit_id_   = commit_id;

        commit_uuid_ = ProfileTraits<Profile>::make_random_snapshot_id();

        superblock_file_pos_ = superblock_file_pos;
        file_size_           = other.file_size_;
        superblock_size_     = other.superblock_size_;

        return allocator_.init(allocator_block_size(other.superblock_size_), 1);
    }

    VoidResult build_superblock_description() noexcept
    {
        return set_description(
            "MEMORIA SWMR MAPPED STORE. VERSION:{}; CommitID:{}, SequenceID:{}, SuperblockFilePos:{}, FileSize:{}",
            0, commit_id_, sequence_id_, superblock_file_pos_, file_size_
        );
    }


    template <typename... Args>
    VoidResult set_description(const char* fmt, Args&&... args) noexcept
    {
        U8String str = format_u8(fmt, std::forward<Args>(args)...);
        if (str.length() < sizeof(magic_buffer_))
        {
            MemCpyBuffer(str.data(), magic_buffer_, str.length());
            magic_buffer_[str.length()] = 0;
        }
        else {
            return MEMORIA_MAKE_GENERIC_ERROR("Supplied SWMR Superblock magic string is too long: {}", str);
        }

        return VoidResult::of();
    }

private:
    static int32_t allocator_block_size(size_t superblock_size) noexcept
    {
        int32_t sb_size = sizeof(SWMRSuperblock);
        int32_t alloc_size = sizeof(PackedAllocator);
        return superblock_size - (sb_size - alloc_size);
    }
};

}
