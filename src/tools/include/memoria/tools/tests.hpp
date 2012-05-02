
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_TOOLS_TESTS_HPP
#define	_MEMORIA_TOOLS_TESTS_HPP


#include <memoria/tools/task.hpp>
#include <memoria/tools/tools.hpp>
#include <memoria/memoria.hpp>

#include <map>
#include <memory>
#include <fstream>

namespace memoria {

using namespace std;



template <typename Profile_, typename Allocator_>
class ProfileTestTask: public TestTask {

public:

	typedef Profile_ 								Profile;
	typedef Allocator_ 								Allocator;


	ProfileTestTask(TaskParametersSet* parameters): TestTask(parameters) {}
	virtual ~ProfileTestTask() throw () {};

	virtual TestReplayParams* CreateTestStep(StringRef name) const						= 0;
	virtual void 			Run(ostream& out)											= 0;
	virtual void 			Replay(ostream& out, TestReplayParams* step_params)			= 0;

	virtual void LoadAllocator(Allocator& allocator, StringRef file_name) const
	{
		unique_ptr <FileInputStreamHandler> in(FileInputStreamHandler::create(file_name.c_str()));
		allocator.load(in.get());
	}

	virtual void LoadAllocator(Allocator& allocator, const TestReplayParams* params) const
	{
		unique_ptr <FileInputStreamHandler> in(FileInputStreamHandler::create(params->GetDumpName().c_str()));
		allocator.load(in.get());
	}

	virtual void StoreAllocator(Allocator& allocator, StringRef file_name) const
	{
		unique_ptr <FileOutputStreamHandler> out(FileOutputStreamHandler::create(file_name.c_str()));
		allocator.store(out.get());
	}



	virtual void Store(Allocator& allocator, TestReplayParams* params) const
	{
		Configure(params);

		String file_name = GetAllocatorFileName(params, ".valid");
		StoreAllocator(allocator, file_name);
		params->SetDumpName(file_name);

		String file_name_invalid = GetAllocatorFileName(params, ".invalid");
		allocator.commit();
		StoreAllocator(allocator, file_name_invalid);

		String props_name = GetPropertiesFileName(params);
		StoreProperties(params, props_name);
	}



	virtual String GetAllocatorFileName(const TestReplayParams* params, StringRef infix = "") const
	{
		return GetResourcePath(params->GetName()+infix+".dump");
	}


};

template <typename T = EmptyType>
class SPTestTaskT: public ProfileTestTask<SmallProfile<>, SmallInMemAllocator> {

	typedef ProfileTestTask<SmallProfile<>, SmallInMemAllocator> Base;

	Int check_count_;

public:
	SPTestTaskT(TaskParametersSet* parameters): Base(parameters), check_count_(0) {}
	virtual ~SPTestTaskT() throw () {};

	virtual TestReplayParams* CreateTestStep(StringRef name) const						= 0;
	virtual void 			Run(ostream& out)											= 0;
	virtual void 			Replay(ostream& out, TestReplayParams* step_params)			= 0;

	void Check(Allocator& allocator, const char* source)
	{
		Int step_count = GetParameters<>()->GetCheckStep();

		if (step_count > 0 && (check_count_ % step_count == 0))
		{
			::memoria::Check<Allocator>(allocator, "Allocator check failed", source);
		}

		check_count_++;
	}

	void Check(Allocator& allocator, const char* message, const char* source)
	{
		Int step_count = GetParameters<>()->GetCheckStep();

		if (check_count_ % step_count == 0)
		{
			::memoria::Check<Allocator>(allocator, message, source);
		}
		check_count_++;
	}

	template <typename CtrType>
	void CheckCtr(CtrType& ctr, const char* message, const char* source)
	{
		Int step_count = GetParameters<>()->GetCheckStep();

		if (step_count > 0 && (check_count_ % step_count == 0))
		{
			::memoria::CheckCtr<CtrType>(ctr, message, source);
		}

		check_count_++;
	}

	template <typename CtrType>
	void CheckCtr(CtrType& ctr, const char* source)
	{
		CheckCtr(ctr, "Container check failed", source);
	}
};

typedef SPTestTaskT<> SPTestTask;




}
#endif
