//
// Copyright(c) 2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "shared/steady_timer.h"
#include "shared/utils.h"
#include "shared/logger/logger.h"
#include "shared/logger/format.h"
#include <signal.h>

#include <stdio.h>
#include <atomic>
#include <thread>
#include <vector>

#define log_error_m   alog::logger().error  (__FILE__, __func__, __LINE__, "LoggerTest")
#define log_warn_m    alog::logger().warn   (__FILE__, __func__, __LINE__, "LoggerTest")
#define log_info_m    alog::logger().info   (__FILE__, __func__, __LINE__, "LoggerTest")
#define log_verbose_m alog::logger().verbose(__FILE__, __func__, __LINE__, "LoggerTest")
#define log_debug_m   alog::logger().debug  (__FILE__, __func__, __LINE__, "LoggerTest")
#define log_debug2_m  alog::logger().debug2 (__FILE__, __func__, __LINE__, "LoggerTest")

using namespace std;
using namespace std::chrono;

namespace  {
int howmany = 1000000;
int threads = 4;
int iters = 5;
bool format_func = true;
const char* log_file = "/tmp/alog-async.log"; // SSD
//const char* log_file = "/mnt/storage/Downloads/alog-async.log"; // HDD
} // namespace

void thread_fun(int howmany)
{
    for (int i = 0; i < howmany; ++i)
    {
        //logger->trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
        if (format_func)
            log_debug2_m << log_format("Hello logger: msg number %?", i);
        else
            log_debug2_m << "Hello logger: msg number " << i;
    }
}

void bench_mt(int howmany, int thread_count, double& delta)
{
    using std::chrono::high_resolution_clock;
    vector<thread> threads;
    auto start = high_resolution_clock::now();

    int msgs_per_thread = howmany / thread_count;
    int msgs_per_thread_mod = howmany % thread_count;
    for (int t = 0; t < thread_count; ++t)
    {
        if (t == 0 && msgs_per_thread_mod)
            threads.push_back(std::thread(thread_fun, msgs_per_thread + msgs_per_thread_mod));
        else
            threads.push_back(std::thread(thread_fun, msgs_per_thread));
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto dlt = high_resolution_clock::now() - start;
    delta = duration_cast<duration<double>>(dlt).count();

    //spdlog::info("Elapsed: {} secs\t {:n}/sec", delta_d, int(howmany / delta_d));
    log_info << log_format("Elapsed: %? secs; %?/sec", delta, int(howmany / delta));
}

int main(int argc, char* argv[])
{
    alog::logger().start();
    alog::logger().addSaverStdOut(alog::Level::Info, false);

    // Создание фильтра для stdout-сэйвера
    {
        alog::FilterModulePtr filter {new alog::FilterModule};
        filter->setName("filter0");
        filter->setMode(alog::Filter::Mode::Exclude);
        filter->addModule("LoggerTest");

        if (alog::SaverPtr saver = alog::logger().findSaver("stdout"))
            saver->addFilter(filter);
    }

    log_info << "ALog speed test is running";
    alog::logger().flush();

    log_info << log_format("-------------------------------------------------");
    log_info << log_format("Messages     : %?", howmany);
    log_info << log_format("Threads      : %?", threads);
    log_info << log_format("Total iters  : %?", iters);
    log_info << log_format("Total iters  : %?", iters);
    log_info << log_format("Use format   : %?", format_func);
    log_info << log_format("-------------------------------------------------");

    std::vector<double> delta_times;
    for (int i = 0; i < iters; ++i)
    {
        // Создаем дефолтный сэйвер для логгера
        bool logContinue = false;
        alog::Level logLevel = alog::Level::Debug2;
        alog::SaverPtr saver {new alog::SaverFile("default",
                                                  log_file,
                                                  logLevel,
                                                  logContinue)};

        alog::FilterModulePtr filter {new alog::FilterModule};
        filter->setName("filter0");
        filter->setMode(alog::Filter::Mode::Include);
        filter->setFilteringNoNameModules(true);
        filter->addModule("LoggerTest");
        saver->addFilter(filter);

        alog::logger().addSaver(saver);

        double delta;
        bench_mt(howmany, threads, delta);

        delta_times.push_back(delta);

        alog::logger().flush(3);
        alog::logger().waitingFlush();
    }

    log_info << log_format("Average time: %? sec", utl::average(delta_times));

    log_info << "ALog test is stopped";
    alog::logger().flush();
    alog::logger().waitingFlush();
    alog::logger().stop();

    return 0;
}

#undef log_error_m
#undef log_warn_m
#undef log_info_m
#undef log_verbose_m
#undef log_debug_m
#undef log_debug2_m
