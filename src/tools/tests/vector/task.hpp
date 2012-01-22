
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_VECTOR_TASK_HPP_
#define MEMORIA_TESTS_VECTOR_TASK_HPP_

#include <memoria/memoria.hpp>

#include <memoria/tools/tests.hpp>

#include "params.hpp"

namespace memoria {

using namespace memoria::vapi;

class VectorTestTask: public SPTestTask {

	typedef StreamContainerTypesCollection::Factory<Vector>::Type 	ByteVector;
	typedef ByteVector::Iterator									BVIterator;

public:

	VectorTestTask(): SPTestTask(new VectorTestTaskParams()) {} //

	virtual ~VectorTestTask() throw() {}


	BigInt GetRandomPosition(ByteVector& array)
	{
		return GetBIRandom(array.Size());
	}

	virtual TestStepParams* CreateTestStep(StringRef name) const
	{
		return new VectorTestStepParams(name);
	}

	virtual void Replay(ostream& out, TestStepParams* step_params)
	{
		VectorTestStepParams* params = static_cast<VectorTestStepParams*>(step_params);
		Allocator allocator;
		LoadAllocator(allocator, params);

//		DoTestStep(out, allocator, params);
	}

	virtual void Run(ostream& out)
	{
		VectorTestStepParams params;
		VectorTestTaskParams* task_params = GetParameters<VectorTestTaskParams>();

		Allocator allocator;
		ByteVector dv(allocator, 1, true);

		try {
			out<<"Insert data"<<endl;

			Int c = 0;
			while (dv.GetSize() < task_params->size_)
			{
				Build(allocator, dv, c + 1, GetRandom(3));
				allocator.commit();
				c++;

				out<<dv.GetSize()<<endl;
			}

			out<<"Remove data. ByteVector contains "<<dv.Size()<<" Mbytes"<<endl;

			for (Int c = 0; ; c++)
			{
				if (!Remove(allocator, dv, GetRandom(3)))
				{
					break;
				}
				allocator.commit();
			}

			allocator.commit();
		}
		catch (...)
		{
			Store(allocator, &params);
			throw;
		}
	}




	void Build(Allocator& allocator, ByteVector& array, UByte value, Int step)
	{
		ArrayData data = CreateRandomBuffer(value, 40*1024);

		if (array.Size() == 0)
		{
			//Insert buffer into an empty array
			auto iter = array.Seek(0);

			iter.Insert(data);

			Check(allocator, "Insertion into an empty array failed. See the dump for details.", MEMORIA_SOURCE);

			auto iter1 = array.Seek(0);
			CheckBufferWritten(iter1, data, "AAA", MEMORIA_SOURCE);
		}
		else {
			if (step == 0)
			{
				//Insert at the start of the array
				auto iter = array.Seek(0);

				BigInt len = array.Size();
				if (len > 100) len = 100;

				ArrayData postfix(len);
				iter.Read(postfix);

				iter.Skip(-len);

				iter.Insert(data);

				Check(allocator, "Insertion at the start of the array failed. See the dump for details.", 	MEMORIA_SOURCE);

				iter.Skip(-data.size());

				CheckBufferWritten(iter, data, "Failed to read and compare buffer from array", 				MEMORIA_SOURCE);
				CheckBufferWritten(iter, postfix, "Failed to read and compare buffer postfix from array", 	MEMORIA_SOURCE);
			}
			else if (step == 1)
			{
				//Insert at the end of the array
				auto iter = array.Seek(array.Size());

				BigInt len = array.Size();
				if (len > 100) len = 100;

				ArrayData prefix(len);
				iter.Skip(-len);
				iter.Read(prefix);

				iter.Insert(data);

				Check(allocator, "Insertion at the end of the array failed. See the dump for details.", MEMORIA_SOURCE);

				iter.Skip(-data.size() - len);

				CheckBufferWritten(iter, prefix, "Failed to read and compare buffer prefix from array", MEMORIA_SOURCE);
				CheckBufferWritten(iter, data, "Failed to read and compare buffer from array", 			MEMORIA_SOURCE);
			}
			else {
				//Insert at the middle of the array

				Int pos = GetRandomPosition(array);
				auto iter = array.Seek(pos);

				if (GetRandom(2) == 0)
				{
					iter.Skip(-iter.data_pos());
					pos = iter.pos();
				}


				BigInt prefix_len = pos;
				if (prefix_len > 100) prefix_len = 100;

				BigInt postfix_len = array.Size() - pos;
				if (postfix_len > 100) postfix_len = 100;

				ArrayData prefix(prefix_len);
				ArrayData postfix(postfix_len);

				iter.Skip(-prefix_len);

				iter.Read(prefix);
				iter.Read(postfix);

				iter.Skip(-postfix.size());

				array.debug() = true;

				iter.Insert(data);

				Check(allocator, "Insertion at the middle of the array failed. See the dump for details.", 	MEMORIA_SOURCE);

				iter.Skip(- data.size() - prefix_len);


				CheckBufferWritten(iter, prefix, 	"Failed to read and compare buffer prefix from array", 	MEMORIA_SOURCE);
				CheckBufferWritten(iter, data, 		"Failed to read and compare buffer from array", 		MEMORIA_SOURCE);
				CheckBufferWritten(iter, postfix, 	"Failed to read and compare buffer postfix from array", MEMORIA_SOURCE);
			}
		}
	}

	bool Remove(Allocator& allocator, ByteVector& array, Int step)
	{
		if (array.Size() < 20000)
		{
			auto iter = array.Seek(0);
			iter.Remove(array.Size());

			Check(allocator, "Remove ByteArray", MEMORIA_SOURCE);
			return array.Size() > 0;
		}
		else {
			BigInt size = GetBIRandom(array.Size() < 40000 ? array.Size() : 40000);

			if (step == 0)
			{
				//Remove at the start of the array
				auto iter = array.Seek(0);

				BigInt len = array.Size() - size;
				if (len > 100) len = 100;

				ArrayData postfix(len);
				iter.Skip(size);
				iter.Read(postfix);
				iter.Skip(-len - size);

				iter.Remove(size);

				Check(allocator, "Removing region at the start of the array failed. See the dump for details.", MEMORIA_SOURCE);

				CheckBufferWritten(iter, postfix, "Failed to read and compare buffer postfix from array", 		MEMORIA_SOURCE);
			}
			else if (step == 1)
			{
				//Remove at the end of the array
				auto iter = array.Seek(array.Size() - size);

				BigInt len = iter.pos();
				if (len > 100) len = 100;

				ArrayData prefix(len);
				iter.Skip(-len);
				iter.Read(prefix);

				iter.Remove(size);

				Check(allocator, "Removing region at the end of the array failed. See the dump for details.", 	MEMORIA_SOURCE);

				iter.Skip(-len);

				CheckBufferWritten(iter, prefix, "Failed to read and compare buffer prefix from array", 		MEMORIA_SOURCE);
			}
			else {
				//Remove at the middle of the array

				Int pos = GetRandom(array.Size() - size);
				auto iter = array.Seek(pos);

				if (GetRandom(2) == 0)
				{
					iter.Skip(-iter.data_pos());
					pos = iter.pos();
				}

				BigInt prefix_len = pos;
				if (prefix_len > 100) prefix_len = 100;

				BigInt postfix_len = array.Size() - (pos + size);
				if (postfix_len > 100) postfix_len = 100;

				ArrayData prefix(prefix_len);
				ArrayData postfix(postfix_len);

				iter.Skip(-prefix_len);

				iter.Read(prefix);

				iter.Skip(size);

				iter.Read(postfix);

				iter.Skip(-postfix.size() - size);

				iter.Remove(size);

				Check(allocator, "Removing region at the middle of the array failed. See the dump for details.", 	MEMORIA_SOURCE);

				iter.Skip(-prefix_len);

				CheckBufferWritten(iter, prefix, 	"Failed to read and compare buffer prefix from array", 			MEMORIA_SOURCE);
				CheckBufferWritten(iter, postfix, 	"Failed to read and compare buffer postfix from array", 		MEMORIA_SOURCE);
			}

			return array.Size() > 0;
		}
	}

};


}


#endif
