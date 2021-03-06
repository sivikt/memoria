
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

#include <memoria/store/swmr/mapped/swmr_mapped_store_common.hpp>

namespace memoria {

template <typename Profile>
class MappedSWMRStoreReadOnlyCommit:
        public MappedSWMRStoreCommitBase<Profile>,
        public EnableSharedFromThis<MappedSWMRStoreReadOnlyCommit<Profile>>
{
    using Base = MappedSWMRStoreCommitBase<Profile>;

    using typename Base::Store;
    using typename Base::CommitDescriptorT;
    using typename Base::CtrID;
    using typename Base::CtrReferenceableResult;
    using typename Base::AllocatorT;

    using typename Base::DirectoryCtrType;

    using Base::directory_ctr_;

    using Base::internal_find_by_root_typed;

public:
    using Base::find;
    using Base::getBlock;

    MappedSWMRStoreReadOnlyCommit(
            MaybeError& maybe_error,
            SharedPtr<Store> store,
            Span<uint8_t> buffer,
            CommitDescriptorT* commit_descriptor
    ) noexcept:
        Base(maybe_error, store, buffer, commit_descriptor)
    {
        wrap_construction(maybe_error, [&]() -> VoidResult {
            auto root_block_id = commit_descriptor->superblock()->directory_root_id();
            if (root_block_id.is_set())
            {
                auto directory_ctr_ref = this->template internal_find_by_root_typed<DirectoryCtrType>(root_block_id);
                MEMORIA_RETURN_IF_ERROR(directory_ctr_ref);
                directory_ctr_ = directory_ctr_ref.get();
            }
            else {
                return MEMORIA_MAKE_GENERIC_ERROR(
                    "Read-only commit {} has no container directory",
                    commit_descriptor->superblock()->commit_id()
                );
            }

            return VoidResult::of();
        });
    }



protected:

    virtual SnpSharedPtr<AllocatorT> self_ptr() noexcept {
        return this->shared_from_this();
    }

};

}
