#ifdef HORIZON_TEST_ENGINE_LOGGING
#include "horizon.h"
#endif
#include "core/util/logging.h"

#ifdef HORIZON_TEST_LEGACY_LOGGING
#include "horizon/core/logging.h"
#endif

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const char *message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

int duplicate_stdout() {
#if defined(_WIN32)
    return _dup(_fileno(stdout));
#else
    return dup(fileno(stdout));
#endif
}

bool restore_stdout(int descriptor) {
#if defined(_WIN32)
    const bool restored = _dup2(descriptor, _fileno(stdout)) == 0;
    _close(descriptor);
#else
    const bool restored = dup2(descriptor, fileno(stdout)) >= 0;
    close(descriptor);
#endif
    return restored;
}

}// namespace

int main(int argc, char**)
{
    const std::filesystem::path output_path =
#ifdef HORIZON_TEST_LEGACY_LOGGING
        std::filesystem::current_path() / (argc > 1 ? "horizon-test-logging-legacy-first.log" : "horizon-test-logging-core-first.log");
#else
        std::filesystem::current_path() / "horizon-test-core-logging.log";
#endif
    std::filesystem::remove(output_path);

    const int original_stdout = duplicate_stdout();
    expect(original_stdout >= 0, "stdout can be duplicated for log capture");
    if (original_stdout < 0) {
        return 1;
    }

    FILE *redirected_stdout = std::freopen(output_path.string().c_str(), "w", stdout);
    expect(redirected_stdout != nullptr, "stdout can be redirected for log capture");
    if (redirected_stdout == nullptr) {
        restore_stdout(original_stdout);
        return 1;
    }

#ifdef HORIZON_TEST_LEGACY_LOGGING
    if (argc > 1)
    {
        Corona::Kernel::CoronaLogger::initialize();
    }
#endif
    horizon::core::log_level_debug();
    horizon::core::debug("debug value ", 1);
    horizon::core::log_level_info();
    horizon::core::info("info value ", 2);
    horizon::core::log_level_warning();
    horizon::core::warning("warning value ", 3);
    horizon::core::info("suppressed info");
    horizon::core::log_level_error();
    horizon::core::warning("suppressed warning");
#ifdef HORIZON_TEST_LEGACY_LOGGING
    horizon::core::log_level_debug();
    Corona::Kernel::CoronaLogger::set_log_level(Corona::Kernel::LogLevel::warning);
    horizon::core::info("suppressed by legacy level");
    horizon::core::log_level_error();
    CFW_LOG_WARNING("suppressed by core level");
    horizon::core::log_level_debug();
    CFW_LOG_DEBUG("legacy formatted value {}", 42);
    CFW_LOG_FLUSH();
#endif
    horizon::core::log_level_debug();
    horizon::core::log_flush();
    std::fflush(stdout);

    expect(restore_stdout(original_stdout), "stdout is restored after log capture");

    std::ifstream output_stream{output_path};
    const std::string output{std::istreambuf_iterator<char>{output_stream},
                             std::istreambuf_iterator<char>{}};
    expect(output.find("debug value 1") != std::string::npos,
           "Debug messages reach the Quill console sink");
    expect(output.find("info value 2") != std::string::npos,
           "Info messages reach the Quill console sink");
    expect(output.find("warning value 3") != std::string::npos,
           "Warning messages reach the Quill console sink");
    expect(output.find("suppressed info") == std::string::npos,
           "Info messages are filtered at Warning level");
    expect(output.find("suppressed warning") == std::string::npos,
           "Warning messages are filtered at Error level");
#ifdef HORIZON_TEST_LEGACY_LOGGING
    expect(output.find("suppressed by legacy level") == std::string::npos,
           "Legacy level changes also filter Core messages");
    expect(output.find("suppressed by core level") == std::string::npos,
           "Core level changes also filter legacy messages");
    expect(output.find("legacy formatted value 42") != std::string::npos,
           "Legacy macros share Core's level and preserve formatting");
    expect(output.find("test_logging.cpp") != std::string::npos,
           "Legacy macros preserve the caller's source location");
#endif
    output_stream.close();
    std::filesystem::remove(output_path);

    bool caught = false;
    try {
        OC_EXCEPTION("logging exception ", 42);
    } catch (const std::runtime_error &exception) {
        caught = true;
        const std::string_view message{exception.what()};
        expect(message.find("logging exception 42") != std::string_view::npos,
               "OC_EXCEPTION preserves serialized arguments");
        expect(message.find("test_logging.cpp") != std::string_view::npos,
               "OC_EXCEPTION preserves the source location");
    }
    expect(caught, "OC_EXCEPTION throws std::runtime_error");

    return failures == 0 ? 0 : 1;
}
