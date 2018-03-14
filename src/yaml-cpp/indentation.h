
// Copyright (c) 2008-2015 Jesse Beder.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <iostream>
#include <cstddef>

#include <memoria/v1/yaml-cpp/ostream_wrapper.h>

namespace memoria {
namespace v1 {
namespace YAML {
struct Indentation {
  Indentation(std::size_t n_) : n(n_) {}
  std::size_t n;
};

inline ostream_wrapper& operator<<(ostream_wrapper& out,
                                   const Indentation& indent) {
  for (std::size_t i = 0; i < indent.n; i++)
    out << ' ';
  return out;
}

struct IndentTo {
  IndentTo(std::size_t n_) : n(n_) {}
  std::size_t n;
};

inline ostream_wrapper& operator<<(ostream_wrapper& out,
                                   const IndentTo& indent) {
  while (out.col() < indent.n)
    out << ' ';
  return out;
}
}}}

