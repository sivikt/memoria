// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)



#include <memoria/v1/process/detail/config.hpp>
#include <system_error>
#include <cstdlib>
#include <boost/winapi/process.hpp>
#include <boost/winapi/get_last_error.hpp>

namespace boost { namespace process { namespace detail { namespace windows {

struct child_handle;

inline void terminate(child_handle &p)
{
    if (!::boost::winapi::TerminateProcess(p.process_handle(), EXIT_FAILURE))
        boost::process::detail::throw_last_error("TerminateProcess() failed");

    ::boost::winapi::CloseHandle(p.proc_info.hProcess);
    p.proc_info.hProcess = ::boost::winapi::INVALID_HANDLE_VALUE_;
}

inline void terminate(child_handle &p, std::error_code &ec) noexcept
{
    if (!::boost::winapi::TerminateProcess(p.process_handle(), EXIT_FAILURE))
        ec = boost::process::detail::get_last_error();
    else
    {
        ec.clear();
        ::boost::winapi::CloseHandle(p.proc_info.hProcess);
        p.proc_info.hProcess = ::boost::winapi::INVALID_HANDLE_VALUE_;
    }
}




}}}}

