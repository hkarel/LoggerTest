//
// Copyright(c) 2015 Gabi Melman.
// Modified     2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <unistd.h>
#include <sys/syscall.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <numeric>
#include <algorithm>

using namespace std;
using namespace std::chrono;
using namespace spdlog;
using namespace spdlog::sinks;
//using namespace utils;

namespace {
int howmany = 1000000;
int queue_size = std::min(howmany + 2, 500000);
int threads = 4;
int iters = 5;
const char* log_file = "/tmp/spdlog-async.log"; // SSD
//const char* log_file = "/mnt/storage/Downloads/spdlog-async.log"; // HDD

} // namespace

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable fopen warning under msvc
#endif                          // _MSC_VER

template<typename T>
typename T::value_type sum(const T& elements)
{
    return std::accumulate(elements.begin(), elements.end(), typename T::value_type(0));
}

template<typename T>
double average(const T& elements)
{
    double s = sum(elements);
    return (s / elements.size());
}

int count_lines(const char *filename)
{
    int counter = 0;
    auto *infile = fopen(filename, "r");
    int ch;
    while (EOF != (ch = getc(infile)))
    {
        if ('\n' == ch)
            counter++;
    }
    fclose(infile);

    return counter;
}

void verify_file(const char *filename, int expected_count)
{
    spdlog::info("Verifying {} to contain {:n} line..", filename, expected_count);
    auto count = count_lines(filename);
    if (count != expected_count)
    {
        spdlog::error("Test failed. {} has {:n} lines instead of {:n}", filename, count, expected_count);
        exit(1);
    }
    spdlog::info("Line count OK ({:n})\n", count);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

void thread_fun(std::shared_ptr<spdlog::logger> logger, int howmany)
{
    pid_t tid = (syscall(SYS_gettid));
    for (int i = 0; i < howmany; ++i)
    {
        logger->trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
        //spdlog::trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
    }
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> logger, int thread_count, double& delta)
{
    using std::chrono::high_resolution_clock;
    vector<thread> threads;
    auto start = high_resolution_clock::now();

    int msgs_per_thread = howmany / thread_count;
    int msgs_per_thread_mod = howmany % thread_count;
    for (int t = 0; t < thread_count; ++t)
    {
        if (t == 0 && msgs_per_thread_mod)
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread + msgs_per_thread_mod));
        else
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread));
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto dlt = high_resolution_clock::now() - start;
    delta = duration_cast<duration<double>>(dlt).count();
    spdlog::info("Elapsed: {} secs\t {}/sec", delta, int(howmany / delta));
}

int main(int argc, char *argv[])
{
    try
    {
        //spdlog::set_pattern("[%^%l%$] %v");
        spdlog::set_pattern("%d.%m.%Y %H:%M:%S.%e %l %v");

        auto slot_size = sizeof(spdlog::details::async_msg);
        spdlog::info("Spdlog speed test is running");
        spdlog::info("-------------------------------------------------");
        spdlog::info("Messages     : {}", howmany);
        spdlog::info("Threads      : {}", threads);
        spdlog::info("Queue        : {} slots", queue_size);
        spdlog::info("Queue memory : {} x {} = {} KB ", queue_size, slot_size, (queue_size * slot_size) / 1024);
        spdlog::info("Total iters  : {}", iters);
        spdlog::info("-------------------------------------------------");

        spdlog::info("");
        spdlog::info("*********************************");
        spdlog::info("Queue Overflow Policy: block");
        spdlog::info("*********************************");

        std::vector<double> delta_times;
        for (int i = 0; i < iters; ++i)
        {
            auto tp = std::make_shared<details::thread_pool>(queue_size, 8);
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);
            file_sink->set_level(level::level_enum::trace);
            auto logger = std::make_shared<async_logger>("", std::move(file_sink), std::move(tp), async_overflow_policy::block);
            logger->set_level(level::level_enum::trace);

            double delta;
            bench_mt(howmany, logger, threads, delta);

            delta_times.push_back(delta);
            // verify_file(filename, howmany);
        }

        spdlog::info("Average time: {} sec", average(delta_times));

//        spdlog::info("");
//        spdlog::info("*********************************");
//        spdlog::info("Queue Overflow Policy: overrun");
//        spdlog::info("*********************************");
//        // do same test but discard oldest if queue is full instead of blocking
//        filename = "/tmp/spdlog-async-overrun.log";
//        for (int i = 0; i < iters; i++)
//        {
//            auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
//            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
//            file_sink->set_level(level::level_enum::trace);
//            auto logger =
//                std::make_shared<async_logger>("", std::move(file_sink), std::move(tp), async_overflow_policy::overrun_oldest);
//            logger->set_level(level::level_enum::trace);
//            bench_mt(howmany, std::move(logger), threads);
//        }

        spdlog::info("Spdlog test is stopped");
        spdlog::shutdown();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
        return 1;
    }
    return 0;
}
