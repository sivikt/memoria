
// Copyright 2015 Victor Smirnov
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

#include <memoria/v1/core/types/types.hpp>
#include <random>
#include <functional>
#include <limits>
#include <iostream>

namespace memoria {
namespace v1 {

//FIXME: this RNGs are single-threaded!

using RngEngine64 = std::mt19937_64;
using RngEngine32 = std::mt19937;


template <typename T, typename Engine>
class RNG {
    Engine engine_;

    std::uniform_int_distribution<T> distribution_;

public:
    RNG(){}

    auto operator()()
    {
        return distribution_(engine_);
    }


    auto operator()(T max)
    {
        std::uniform_int_distribution<T> distribution(0, max > 0 ? max - 1 : 0);
        return distribution(engine_);
    }

    Engine& engine() {
        return engine_;
    }

    const Engine& engine() const {
        return engine_;
    }

    void seed(T value) {
        std::seed_seq ss({value});
        engine_.seed(ss);
    }
};

using RngInt    = RNG<Int, RngEngine32>;
using RngBigInt = RNG<BigInt, RngEngine64>;


RngInt& getGlobalIntGenerator();
RngBigInt& getGlobalBigIntGenerator();

Int     getRandomG();
Int     getRandomG(Int max);
void    Seed(Int value);
Int     getSeed();

BigInt  getBIRandomG();
BigInt  getBIRandomG(BigInt max);
void    SeedBI(BigInt value);
BigInt  getSeedBI();


Int getNonZeroRandomG(Int size);


}}