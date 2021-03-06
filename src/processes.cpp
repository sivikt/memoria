
// Copyright 2018 Victor Smirnov
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


#include <memoria/reactor/process.hpp>
#include <memoria/reactor/application.hpp>

#include <memoria/core/tools/type_name.hpp>
#include <memoria/core/memory/ptr_cast.hpp>

#include <boost/optional/optional_io.hpp>

#include <iostream>

using namespace memoria::v1;
using namespace memoria::reactor;


int main(int argc, char** argv, char** envp)
{
    return Application::run_e(argc, argv, envp, []{
        ShutdownOnScopeExit hh;

        auto env_map = app().env().entries();

        Process process = ProcessBuilder::create("print_env")
                .with_env(env_map)
                .with_vfork(true)
                .run();

        auto out = process.out_stream();

        while (!out.is_closed())
        {
            uint8_t buf[200];
            std::memset(buf, 0, sizeof(buf));

            size_t read = out.read(buf, sizeof(buf) - 1);

            if (read > 0) {
                engine().cout("{}", ptr_cast<const char>(buf)) << std::flush;
            }
        }

        process.terminate();

        engine().coutln("Joining process {}", "");
        process.join();

        engine().coutln("Exit. Status: {}, exit code: {}", process.status(), process.exit_code());

        return 0;
    });
}
