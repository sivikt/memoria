// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <memoria/core/tools/cow_tree/cow_tree.hpp>
#include <memoria/core/tools/time.hpp>
#include <memoria/core/tools/random.hpp>

#include <vector>

using namespace memoria;
using namespace memoria::cow::tree;
using namespace std;

int main(int argc, const char** argv, const char** envp)
{
	CoWTree<BigInt, BigInt> tree;

	Int size = 1000000;

	vector<BigInt> keys(size);

	BigInt tr0 = getTimeInMillis();

	SeedBI(0xaa123bb213ce);

	for (auto& k: keys)
	{
		k = getBIRandomG();
	}

	cout<<"Random array created in "<<FormatTime(getTimeInMillis() - tr0)<<endl;

	BigInt t0 = getTimeInMillis();

	auto txn = tree.transaction();

	for (int c = 0; c < size; c++)
	{
		tree.assign(txn, keys[c], c);
	}

	cout<<"Tree created in: "<<FormatTime(getTimeInMillis() - t0)<<endl;
	cout<<keys.size()<<" "<<tree.size(txn)<<endl;

	txn.commit();

	auto sn = tree.snapshot();

	long tf0 = getTimeInMillis();

	long finds = 0;

	for (Int c = 0; c < size; c++)
	{
		auto key = keys[c];

		auto op = tree.find(sn, key + 1);

		finds += op;
	}


	long tf1 = getTimeInMillis();

	for (Int c = 0; c < size; c++)
	{
		auto key = keys[c];

		auto op = tree.find(sn, key);

		finds += op;
	}

	long tf2 = getTimeInMillis();

	cout<<"Find time: "<<FormatTime(tf1-tf0)<<" -- "<<FormatTime(tf2-tf1)<<" finds="<<finds<<endl;

	Int cnt = 0;
	for (auto iter = tree.begin(sn); !iter.is_end(); iter++)
	{
		cnt++;
	}

	cout<<"FW Cnt = "<<cnt<<endl;

	cnt = 0;

	for (auto iter = tree.rbegin(sn); !iter.is_start(); iter--)
	{
		cnt++;
	}

	cout<<"BW Cnt = "<<cnt<<endl;





	long td0 = getTimeInMillis();

	auto tx2 = tree.transaction();

	BigInt removed = 0;

	for (const auto& k: keys)
	{
		if (tree.remove(tx2, k))
		{
			removed++;
		}
		else {
			cout<<"Didn't remove "<<k<<endl;
		}
	}

	cout<<"After deletion = "<<tree.size(tx2)<<", removed = "<<removed<<", time = "<<FormatTime(getTimeInMillis() - td0)<<endl;

	tree.begin(tx2).dump();

	tx2.commit();


	auto sn2 = tree.snapshot();

	cout<<"After deletion, snapshot  = "<<tree.size(sn2)<<", time = "<<FormatTime(getTimeInMillis() - td0)<<endl;

	tree.dump_log();
}
