//
// Copyright(c) 2015 Gabi Melman.
// Modified     2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "params_test.h"
#include "hw_monitor.h"

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


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable fopen warning under msvc
#endif                          // _MSC_VER

//int count_lines(const char *filename)
//{
//    int counter = 0;
//    auto *infile = fopen(filename, "r");
//    int ch;
//    while (EOF != (ch = getc(infile)))
//    {
//        if ('\n' == ch)
//            counter++;
//    }
//    fclose(infile);

//    return counter;
//}

//void verify_file(const char *filename, int expected_count)
//{
//    spdlog::info("Verifying {} to contain {:n} line..", filename, expected_count);
//    auto count = count_lines(filename);
//    if (count != expected_count)
//    {
//        spdlog::error("Test failed. {} has {:n} lines instead of {:n}", filename, count, expected_count);
//        exit(1);
//    }
//    spdlog::info("Line count OK ({:n})\n", count);
//}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static void thread_func(std::shared_ptr<spdlog::logger> logger, int howmany)
{
    pid_t tid = (syscall(SYS_gettid));
    for (int i = 0; i < howmany; ++i)
    {
        logger->trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
        //spdlog::trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
    }
}

static void bench_mt(std::shared_ptr<spdlog::logger> logger, const TestParams& params)
{
    vector<thread> threads;
    int msgs_per_thread = params.howmany / params.threads;
    int msgs_per_thread_mod = params.howmany % params.threads;

    for (int t = 0; t < (params.threads - 1); ++t)
        threads.push_back(std::thread(thread_func, logger, msgs_per_thread));

    if (msgs_per_thread_mod)
        thread_func(logger, msgs_per_thread + msgs_per_thread_mod);
    else
        thread_func(logger, msgs_per_thread);

    for (auto& t : threads)
        t.join();
}

void spdlog_test(const TestParams& params)
{
    using std::chrono::high_resolution_clock;

    try
    {
        //spdlog::set_pattern("[%^%l%$] %v");
        spdlog::set_pattern("%d.%m.%Y %H:%M:%S.%e %l %v");

        HwMonitor hwmon;
        uint32_t start_mem = hwmon.procMem();

        auto slot_size = sizeof(spdlog::details::async_msg);
        spdlog::info("Spdlog speed test is running");
        spdlog::info("-------------------------------------------------");
        spdlog::info("Messages     : {}", params.howmany);
        spdlog::info("Threads      : {}", params.threads);
        spdlog::info("Queue        : {} slots", params.queue_size);
        spdlog::info("Queue memory : {} x {} = {} KB ", params.queue_size, slot_size, (params.queue_size * slot_size) / 1024);
        spdlog::info("Total iters  : {}", params.iters);
        spdlog::info("Start memory : {} MB", start_mem);
        spdlog::info("-------------------------------------------------");

        spdlog::info("");
        spdlog::info("*********************************");
        spdlog::info("Queue Overflow Policy: block");
        spdlog::info("*********************************");

        std::vector<double> delta1_times;
        std::vector<double> delta2_times;

        std::vector<int> count1;
        std::vector<int> count2;

        bool first_test = true;

        for (int i = 0; i < params.iters; ++i)
        {
            std::vector<uint32_t> cpu_load;
            std::vector<uint32_t> mem_load;
            atomic_bool test_complete = {false};

            auto hwmon_func = [&]()
            {
                while (true)
                {
                    hwmon.cpuLoad();
                    cpu_load.push_back(hwmon.procLoad());
                    mem_load.push_back(hwmon.procMem());
                    usleep(20*1000);
                    if (test_complete)
                        break;
                }
            };
            thread t1 {hwmon_func}; (void) t1;

            auto start0 = high_resolution_clock::now();

            auto tp = std::make_shared<details::thread_pool>(params.queue_size, 4);
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(params.spdlog_file, true);
            file_sink->set_level(level::level_enum::trace);
            auto logger = std::make_shared<async_logger>("", file_sink, std::move(tp), async_overflow_policy::block);
            logger->set_level(level::level_enum::trace);

            auto dlt0 = high_resolution_clock::now() - start0;
            double delta0 = duration_cast<duration<double>>(dlt0).count();

            uint32_t begin_alloc_mem = hwmon.procMem() - start_mem;

            auto start = high_resolution_clock::now();

            bench_mt(logger, params);

            auto dlt1 = high_resolution_clock::now() - start;
            double delta1 = duration_cast<duration<double>>(dlt1).count();

            logger->flush();
            logger.reset();

            file_sink->flush();
            file_sink.reset();

            auto dlt2 = high_resolution_clock::now() - start;
            double delta2 = duration_cast<duration<double>>(dlt2).count();

            test_complete = true;
            t1.join();

            int cnt1 = int(params.howmany / delta1);
            int cnt2 = int(params.howmany / delta2);

            if (first_test) {
            spdlog::info("Begin alloc mem   {} MB", begin_alloc_mem);
            first_test = false;
            }
            spdlog::info("Elapsed (create)  {} secs", delta0);
            spdlog::info("Elapsed (logging) {} secs\t {}/sec", delta1, cnt1);
            spdlog::info("Elapsed (flush)   {} secs\t {}/sec", delta2, cnt2);

            uint32_t mem_max = max(mem_load) - start_mem;
            uint32_t mem_average = average(mem_load) - start_mem;
            spdlog::info("Memory usage max: {} MB; average: {} MB", mem_max, mem_average);

            uint32_t cpu_max = max(cpu_load);
            uint32_t cpu_average = average(cpu_load);
            spdlog::info("CPU usage    max: {} %; average: {} %", cpu_max, cpu_average);

            spdlog::info("---");

            delta1_times.push_back(delta1);
            delta2_times.push_back(delta2);

            count1.push_back(cnt1);
            count2.push_back(cnt2);

            sleep(3);

            // verify_file(filename, howmany);
        }

        spdlog::info("Average (logging) {} secs\t {}/sec", average(delta1_times), int(average(count1)));
        spdlog::info("Average (flush)   {} secs\t {}/sec", average(delta2_times), int(average(count2)));

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

        spdlog::info("Spdlog test is stopped\n");
        spdlog::shutdown();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
    }
}
