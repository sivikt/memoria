
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

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

// The following ifdef block is the standard way of creating macros which make
// exporting from a DLL simpler. All files within this DLL are compiled with the
// yaml_cpp_EXPORTS symbol defined on the command line. This symbol should not
// be defined on any project that uses this DLL. This way any other project
// whose source files include this file see YAML_CPP_API functions as being
// imported from a DLL, whereas this DLL sees symbols defined with this macro as
// being exported.
#undef YAML_CPP_API

#ifdef YAML_CPP_DLL      // Using or Building YAML-CPP DLL (definition defined
                         // manually)
#ifdef yaml_cpp_EXPORTS  // Building YAML-CPP DLL (definition created by CMake
                         // or defined manually)
//	#pragma message( "Defining YAML_CPP_API for DLL export" )
#define YAML_CPP_API __declspec(dllexport)
#else  // yaml_cpp_EXPORTS
//	#pragma message( "Defining YAML_CPP_API for DLL import" )
#define YAML_CPP_API __declspec(dllimport)
#endif  // yaml_cpp_EXPORTS
#else   // YAML_CPP_DLL
#define YAML_CPP_API
#endif  // YAML_CPP_DLL

