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


#pragma once

#include <memoria/v1/core/tools/strings/string.hpp>
#include <memoria/v1/core/types/types.hpp>
#include <memoria/v1/core/tools/time.hpp>

namespace memoria {
namespace v1 {



class Ticker {
	size_t threshold_value_;
	size_t threshold_;
	size_t ticks_ = 0;
	BigInt start_time_;
	BigInt last_time_;
	BigInt threshold_time_;

public:
	Ticker(size_t threshold): threshold_value_(threshold), threshold_(threshold - 1)
	{
		threshold_time_ = last_time_ = start_time_ = getTimeInMillis();
	}

	bool is_threshold()
	{
		if (threshold_ != ticks_) {
			return false;
		}

		threshold_time_ = getTimeInMillis();

		return true;
	}

	void next()
	{
		last_time_ = threshold_time_;
		threshold_ += threshold_value_;
	}

	BigInt duration() const {
		return threshold_time_ - last_time_;
	}

	size_t ticks() const {return ticks_;}
	size_t size()  const {return threshold_value_;}

	BigInt start_time() const {return start_time_;}

	void tick() {
		ticks_++;
	}
};


}}





