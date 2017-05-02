// Copyright 2016 Victor Smirnov
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



#include <memoria/v1/memoria.hpp>
#include <memoria/v1/containers/map/map_factory.hpp>
#include <memoria/v1/core/tools/iobuffer/io_buffer.hpp>

#include <memoria/v1/core/tools/strings/string_codec.hpp>
#include <memoria/v1/core/tools/time.hpp>

#include <algorithm>
#include <vector>
#include <type_traits>

using namespace memoria::v1;
using namespace memoria::v1::btss;
using namespace std;

class PrintingConsumer: public bt::BufferConsumer<IOBuffer> {

    IOBuffer buffer_;

public:
    PrintingConsumer(size_t size): buffer_(size) {}

    virtual IOBuffer& buffer() {
        return buffer_;
    }

    virtual int32_t process(IOBuffer& buffer, int32_t entries)
    {
        for (int32_t c = 0; c < entries; c++)
        {
            cout << "Key: '" << buffer.getString() << "' Value: '" << buffer.getString() << "'" << endl;
        }

        return entries;
    }
};

template <typename Iter>
class MapBufferProducer: public BufferProducer<IOBuffer> {
    using Base = BufferProducer<IOBuffer>;

    Iter iter_;
    Iter end_;

    using Key   = std::decay_t<decltype(std::declval<Iter>()->key())>;
    using Value = std::decay_t<decltype(std::declval<Iter>()->value())>;

    IOBuffer buffer_;

public:
    MapBufferProducer(const Iter& begin, const Iter& end, size_t buffer_size):
        iter_(begin), end_(end), buffer_(buffer_size)
    {}

    virtual IOBuffer& buffer() {return buffer_;}

    virtual int32_t populate(IOBuffer& buffer)
    {
        int32_t entries = 0;

        ValueCodec<Key>     key_codec;
        ValueCodec<Value>   value_codec;

        while (iter_ != end_)
        {
            size_t key_len = key_codec.length(iter_->key());

            if (buffer.has_capacity(key_len))
            {
                key_codec.encode(buffer.array(), iter_->key(), buffer.pos());
                buffer.skip(key_len);
            }
            else {
                return entries;
            }

            size_t value_len = value_codec.length(iter_->value());

            if (buffer.has_capacity(value_len))
            {
                value_codec.encode(buffer.array(), iter_->value(), buffer.pos());
                buffer.skip(value_len);
            }
            else {
                return entries;
            }

            entries++;
            iter_++;
        }

        return -entries;
    }
};


template <typename Key, typename Value>
class KeyValuePair {
    using MyType = KeyValuePair<Key, Value>;

    Key key_;
    Value value_;
public:
    KeyValuePair() {}
    KeyValuePair(const Key& key, const Value& value):
        key_(key), value_(value)
    {}

    const Key& key() const {return key_;}
    const Value& value() const {return value_;}

    void swap(MyType& other)
    {
        std::swap(key_, other.key_);
        std::swap(value_, other.value_);
    }

    bool operator<(const MyType& other) const
    {
        return key_ < other.key_;
    }
};


int main()
{
    MEMORIA_INIT(DefaultProfile<>);

    using Key   = String;
    using Value = String;

    DInit<Map<Key, Value>>();

    try {
        auto alloc = PersistentInMemAllocator<>::create();

        auto snp = alloc->master()->branch();

        auto map = create<Map<Key, Value>>(snp);

        int size = 10000000;

        using PairVector = vector<KeyValuePair<Key, Value>>;

        PairVector pairs;

        int64_t ts0 = getTimeInMillis();

        for (int c = 0; c < size; c++)
        {
            pairs.emplace_back("key_" + toString(c), "value_" + toString(c));
        }

        int64_t ts1 = getTimeInMillis();

        std::sort(pairs.begin(), pairs.end());

        int64_t ts2 = getTimeInMillis();

        cout << "Vector creation: " << FormatTime(ts1 - ts0) << ", sorting: " << FormatTime(ts2 - ts1) << endl;

        MapBufferProducer<PairVector::const_iterator> producer(pairs.begin(), pairs.end(), 65536);

        int64_t t0 = getTimeInMillis();
        map->begin()->insert_iobuffer(&producer);

        cout << "Insertion time: " << FormatTime(getTimeInMillis() - t0) << endl;

//        FSDumpAllocator(snp, "map_full.dir");


//        PrintingConsumer consumer(65536);

//        auto iter = map->begin();
//
//        iter->read_buffer(&consumer);

        // Finish snapshot so no other updates are possible.
        snp->commit();

        // Store binary contents of allocator to the file.
        auto out = FileOutputStreamHandler::create("map_data.dump");
        alloc->store(out.get());
    }
    catch (Exception& ex)
    {
        cout << ex.message() << " at " << ex.source() << endl;
    }

    // Destroy containers metadata.
    MetadataRepository<DefaultProfile<>>::cleanup();
}