
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

#include <memoria/tests/runner.hpp>
#include <memoria/reactor/process.hpp>
#include <memoria/reactor/pipe_streams_reader.hpp>
#include <memoria/reactor/application.hpp>
#include <memoria/reactor/file_streams.hpp>
#include <memoria/filesystem/operations.hpp>

#include <memoria/core/tools/random.hpp>
#include <memoria/core/tools/time.hpp>

#include <yaml-cpp/yaml.h>

#include <memoria/core/packed/tools/packed_allocator_types.hpp>

#include <sstream>

namespace memoria {
namespace tests {

using filesystem::path;
using namespace reactor;

void dump_exception(std::ostream& out, std::exception_ptr& ex)
{
    try {
        std::rethrow_exception(ex);
    }
    catch (MemoriaThrowable& th) {
        th.dump(out);
    }
    catch (std::exception& th) {
        out << "STD Exception: " << th.what() << std::endl;
    }
    catch (boost::exception& th)
    {
        out << boost::diagnostic_information(th);
        out << std::flush;
    }
    catch (...) {
        out << "Unknown exception" << std::endl;
    }
}

namespace {

filesystem::path get_tests_config_path()
{
    auto& app = reactor::app();

    filesystem::path config_path;

    if (app.options().count("config") > 0)
    {
        config_path = app.options()["config"].as<std::string>();
    }
    else {
        auto program_path = reactor::get_program_path();
        config_path = program_path.parent_path();
        config_path.append(get_image_name().string() + ".yaml");
    }

    return config_path;
}


filesystem::path get_tests_output_path()
{
    auto& app = reactor::app();

    filesystem::path output_path;

    if (app.options().count("output") > 0)
    {
        output_path = app.options()["output"].as<std::string>();
    }
    else {
        auto image_name = get_image_name();
        output_path = image_name.string() + ".out";
    }

    return output_path;
}

U8String get_test_coverage_str()
{
    if (app().options().count("coverage") > 0) {
        return U8String(app().options()["coverage"].as<std::string>());
    }
    else {
        return "small";
    }
}

Optional<TestCoverage> get_test_coverage()
{
    if (app().options().count("coverage") > 0) {
        return coverage_from_string(get_test_coverage_str().to_u8());
    }
    else {
        return TestCoverage::SMALL;
    }
}


}



TestStatus run_single_test(const U8String& test_path)
{
    auto test = tests_registry().find_test(test_path);
    if (test)
    {
        filesystem::path output_dir_base = get_tests_output_path();
        filesystem::path config_path     = get_tests_config_path();

        U8String suite_name;
        U8String test_name;

        std::tie(suite_name, test_name) = TestsRegistry::split_path(test_path);

        YAML::Node test_config;

        int64_t seed = getTimeInMillis();

        if (filesystem::is_regular_file(config_path))
        {
            FileInputStream<char> fi_stream(open_buffered_file(config_path, FileFlags::RDONLY));
            YAML::Node config = YAML::Load(fi_stream);

            if (config["seed"])
            {
                seed = config["seed"].as<int64_t>();
            }

            YAML::Node suite_node = config[suite_name.to_u8().to_std_string()];
            if (suite_node)
            {
                test_config = suite_node[test_name.to_u8().to_std_string()];
            }
        }

        reactor::engine().coutln("seed = {}", seed);

        Seed(seed);
        SeedBI(seed);

        filesystem::path test_output_dir = output_dir_base;
        test_output_dir.append(suite_name.to_u8().to_std_string());
        test_output_dir.append(test_name.to_u8().to_std_string());

		filesystem::create_directories(test_output_dir);

        Optional<TestCoverage> coverage = get_test_coverage();

        DefaultTestContext ctx(
            test_config,
            config_path.parent_path(),
            filesystem::absolute(test_output_dir),
            coverage.get(),
            false,
            seed
        );

        int64_t start_time = getTimeInMillis();
        test.get().run(&ctx);
        int64_t end_time = getTimeInMillis();

        if (ctx.status() == TestStatus::PASSED)
        {
            reactor::engine().coutln("PASSED in {}s", FormatTime(end_time - start_time));
        }
        else {
            reactor::engine().coutln("FAILED in {}s", FormatTime(end_time - start_time));
            if (ctx.ex())
            {
                dump_exception(ctx.out(), ctx.ex());
            }
        }

        return ctx.status();
    }
    else {
        MMA_THROW(TestConfigurationException()) << format_ex("No test with path '{}' found", test_path);
    }
}


TestStatus replay_single_test(const U8String& test_path)
{
    auto test = tests_registry().find_test(test_path);
    if (test)
    {
        filesystem::path output_dir_base = get_tests_output_path();

        U8String suite_name;
        U8String test_name;

        std::tie(suite_name, test_name) = TestsRegistry::split_path(test_path);

        filesystem::path test_output_dir = output_dir_base;
        test_output_dir.append(suite_name.to_u8().to_std_string());
        test_output_dir.append(test_name.to_u8().to_std_string());

        filesystem::path config_path = test_output_dir;
        config_path.append("config.yaml");

        YAML::Node config;

        int64_t seed = getTimeInMillis();

        if (filesystem::is_regular_file(config_path))
        {
            reactor::FileInputStream<char> fi_config(open_buffered_file(config_path, FileFlags::RDONLY));
            config = YAML::Load(fi_config);

            if (config["seed"])
            {
                seed = config["seed"].as<int64_t>();
            }


        }

        reactor::engine().coutln("seed = {}", seed);
        Seed(static_cast<int32_t>(seed));
        SeedBI(seed);

        Optional<TestCoverage> coverage = get_test_coverage();

        DefaultTestContext ctx(
            config,
            config_path.parent_path(),
            filesystem::absolute(test_output_dir),
            coverage.get(),
            true,
            seed
        );

        int64_t start_time = getTimeInMillis();
        test.get().run(&ctx);
        int64_t end_time = getTimeInMillis();

        if (ctx.status() == TestStatus::PASSED)
        {
            reactor::engine().coutln("PASSED in {}s", FormatTime(end_time - start_time));
        }
        else {
            reactor::engine().coutln("FAILED in {}s", FormatTime(end_time - start_time));
            if (ctx.ex())
            {
                dump_exception(ctx.out(), ctx.ex());
            }
        }

        return ctx.status();
    }
    else {
        MMA_THROW(TestConfigurationException()) << format_ex("No test with path '{}' found", test_path);
    }
}



template <typename T>
U8String to_string(const std::vector<T>& array)
{
    U8String data = "[";

    bool first = true;

    for (const auto& ee : array)
    {
        if (!first) {
            data += ", ";
        }
        else {
            first = false;
        }

        std::stringstream ss;
        ss << ee;

        data += U8String(ss.str());
    }

    data += "]";
    return data;
}

namespace  {

enum class EnablementType {DISABLED, ENABLED, DEFAULT};

EnablementType is_enabled(YAML::Node& node)
{
    if (node["enable"]) {
        return node["enable"].as<bool>() ? EnablementType::ENABLED : EnablementType::DISABLED;
    }

    return EnablementType::DEFAULT;
}

bool is_test_enabled(EnablementType global, EnablementType suite, EnablementType test)
{
    if (test == EnablementType::ENABLED) {
        return true;
    }
    else if (test == EnablementType::DISABLED) {
        return false;
    }
    else if (suite == EnablementType::ENABLED) {
        return true;
    }
    else if (suite == EnablementType::DISABLED) {
        return false;
    }
    else if (global == EnablementType::ENABLED) {
        return true;
    }
    else if (global == EnablementType::DISABLED) {
        return false;
    }
    else {
        return true; // enabled by default
    }
}


}

void run_tests()
{
    filesystem::path output_dir_base = get_tests_output_path();

    filesystem::path config_path    = get_tests_config_path();
    U8String config_file           = U8String(config_path.string());

    YAML::Node tests_config;

    if (filesystem::is_regular_file(config_path))
    {
        reactor::FileInputStream<char> config(open_buffered_file(config_path, FileFlags::RDONLY));
        tests_config = YAML::Load(config);
    }

    auto global_enabled = is_enabled(tests_config);
    const auto& suites  = tests_registry().suites();

    for (auto& suite: suites)
    {
        auto suite_node     = tests_config[suite.first.to_u8().data()];
        auto suite_enabled  = is_enabled(suite_node);

        std::vector<U8String> failed;
        std::vector<U8String> crashed;
		
        int32_t tests_run{};
        int32_t passed{};

        for (auto& test: suite.second->tests())
        {
            auto test_node     = suite_node[test.first.to_u8().data()];
            auto test_enabled  = is_enabled(test_node);

            if (is_test_enabled(global_enabled, suite_enabled, test_enabled))
            {
                tests_run++;

                filesystem::path test_output_dir = output_dir_base;
                test_output_dir.append(suite.first.to_u8().to_std_string());
                test_output_dir.append(test.first.to_u8().to_std_string());

                filesystem::create_directories(test_output_dir);

                U8String test_path = suite.first + "/" + test.first;

                std::vector<U8String> args;

                args.emplace_back("tests2");
                args.emplace_back("--test");
                args.emplace_back(test_path);

                if (!config_file.is_empty()) {
                    args.emplace_back("--config");
                    args.emplace_back(config_file);
                }

                if (test_node["threads"])
                {
                    args.emplace_back("-t");
                    args.emplace_back(test_node["threads"].as<std::string>());
                }
                else {
                    auto state = test.second->create_state();
                    args.emplace_back("-t");
                    args.emplace_back(std::to_string(state->threads()));
                }

                args.emplace_back("--output");
                args.emplace_back(U8String(output_dir_base.string()));

                args.emplace_back("--coverage");
                args.emplace_back(get_test_coverage_str());

                reactor::Process process = reactor::ProcessBuilder::create(reactor::get_program_path())
                        .with_args(args)
                        .run();

                filesystem::path std_output = test_output_dir;
                std_output.append("stdout.txt");
                File out_file = open_buffered_file(std_output, FileFlags::CREATE | FileFlags::RDWR | FileFlags::TRUNCATE);

                filesystem::path std_error = test_output_dir;
                std_error.append("stderr.txt");
                File err_file = open_buffered_file(std_error, FileFlags::CREATE | FileFlags::RDWR | FileFlags::TRUNCATE);

                reactor::InputStreamReaderWriter out_reader(process.out_stream(), out_file.ostream());
                reactor::InputStreamReaderWriter err_reader(process.err_stream(), err_file.ostream());

                process.join();

                out_reader.join();
                err_reader.join();

                out_file.close();
                err_file.close();

                auto status = process.status();

                if (!(status == reactor::Process::Status::EXITED && process.exit_code() == 0))
                {
                    if (status == reactor::Process::Status::CRASHED)
                    {
                        crashed.push_back(test.first);
                    }
                    else {
                        failed.push_back(test.first);
                    }
                }
                else {
                    passed++;
                }
            }
        }

        if (tests_run > 0)
        {
            if (failed.size() == 0 && crashed.size() == 0)
            {
                reactor::engine().coutln("{}: PASSED ({})", suite.first, passed);
            }
            else if (failed.size() > 0 && crashed.size() > 0)
            {
                reactor::engine().coutln("{}: PASSED ({}); FAILED {}; CRASHED {}", suite.first, passed, to_string(failed), to_string(crashed));
            }
            else if (failed.size() > 0) {
                reactor::engine().coutln("{}: PASSED ({}); FAILED {}", suite.first, passed, to_string(failed));
            }
            else {
                reactor::engine().coutln("{}: PASSED ({}); CRASHED {}", suite.first, passed, to_string(crashed));
            }
        }
    }
}

void Test::run(TestContext *context) noexcept
{
    std::unique_ptr<TestState> state;

    try {
        state = create_state();
        state->set_seed(context->seed());
        state->set_replay(context->is_replay());

        state->working_directory_ = context->data_directory();

        state->pre_configure(context->coverage());
        state->add_field_handlers();
        state->add_indirect_field_handlers();
        state->post_configure(context->coverage());

        CommonConfigurationContext configuration_context(context->configurator()->config_base_path());

        state->internalize(context->configurator()->configuration(), &configuration_context);

        state->set_up();
    }
    catch (...) {
        context->failed(TestStatus::SETUP_FAILED, std::current_exception(), state.get());
        return;
    }

    try {
        if (context->is_replay()) {
            replay_test(state.get());
        }
        else {
            test(state.get());
        }

        state->tear_down();
        context->passed();
    }
    catch (...) {
        context->failed(TestStatus::TEST_FAILED, std::current_exception(), state.get());
    }
}

void Test::replay_test(TestState* state) {
    MMA_THROW(TestException()) << WhatCInfo("No Replay method exists for the test requested");
}

void DefaultTestContext::failed(TestStatus detail, std::exception_ptr ex, TestState* state) noexcept
{
    status_ = detail;
    ex_ = ex;

    if (detail == TestStatus::TEST_FAILED && !state->is_replay())
    {
        state->on_test_failure();

        filesystem::path config_path = this->data_directory();
        config_path.append("config.yaml");

        CommonConfigurationContext configuration_context(data_directory());
        YAML::Node config;
        state->externalize(config, &configuration_context);

        FileOutputStream<char> stream(open_buffered_file(config_path, FileFlags::RDWR | FileFlags::CREATE | FileFlags::TRUNCATE));
        YAML::Emitter emitter(stream);

        emitter.SetIndent(4);
        emitter << config;

        stream.flush();
    }
}

}}
