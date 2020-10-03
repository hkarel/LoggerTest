//
// Copyright(c) 2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "params_test.h"
#include "hw_monitor.h"

#include "shared/logger/logger.h"
#include "shared/logger/format.h"

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <atomic>
#include <thread>
#include <vector>

#define log_error_m   alog::logger().error   (alog_line_location, "LoggerTest")
#define log_warn_m    alog::logger().warn    (alog_line_location, "LoggerTest")
#define log_info_m    alog::logger().info    (alog_line_location, "LoggerTest")
#define log_verbose_m alog::logger().verbose (alog_line_location, "LoggerTest")
#define log_debug_m   alog::logger().debug   (alog_line_location, "LoggerTest")
#define log_debug2_m  alog::logger().debug2  (alog_line_location, "LoggerTest")

using namespace std;
using namespace std::chrono;

static void thread_func(int howmany, bool format_func)
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

static void bench_mt(const TestParams& params)
{
    vector<thread> threads;
    int msgs_per_thread = params.howmany / params.threads;
    int msgs_per_thread_mod = params.howmany % params.threads;

    for (int t = 0; t < params.threads; ++t)
    {
        if (t == 0 && msgs_per_thread_mod)
            threads.push_back(std::thread(thread_func, msgs_per_thread + msgs_per_thread_mod, params.format_func));
        else
            threads.push_back(std::thread(thread_func, msgs_per_thread, params.format_func));
    }

    for (auto& t : threads)
        t.join();
}

void alog_test(const TestParams& params)
{
    using std::chrono::high_resolution_clock;

    alog::logger().start();
    //alog::logger().addSaverStdOut(alog::Level::Info, false);
    alog::logger().addSaverStdOut(alog::Level::Debug2, false);

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

    HwMonitor hwmon;
    uint32_t start_mem = hwmon.procMem();

    log_info << log_format("-------------------------------------------------");
    log_info << log_format("Messages     : %?"   , params.howmany);
    log_info << log_format("Threads      : %?"   , params.threads);
    log_info << log_format("Total iters  : %?"   , params.iters);
    log_info << log_format("Use format   : %?"   , params.format_func);
    log_info << log_format("Start memory : %? MB", start_mem);
    log_info << log_format("-------------------------------------------------");

    alog::logger().flush();
    alog::logger().waitingFlush();

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

        // Создаем дефолтный сэйвер для логгера
        bool logContinue = false;
        alog::Level logLevel = alog::Level::Debug2;
        alog::SaverPtr saver {new alog::SaverFile("default",
                                                  params.alog_file,
                                                  logLevel,
                                                  logContinue)};

        alog::FilterModulePtr filter {new alog::FilterModule};
        filter->setName("filter0");
        filter->setMode(alog::Filter::Mode::Include);
        filter->setFilteringNoNameModules(true);
        filter->addModule("LoggerTest");
        saver->addFilter(filter);

        alog::logger().addSaver(saver);

        uint32_t begin_alloc_mem = hwmon.procMem() - start_mem;

        auto start = high_resolution_clock::now();

        /**
          Не отключаем сейвер вывода в консоль, пусть в тесте работает механизм
          фильтрации сообщений по имени модуля. Показатели  теста  будут  хуже,
          но такой вариант ближе к обычному режиму работы логгера
        */
        //if (alog::SaverPtr saver = alog::logger().findSaver("stdout"))
        //    saver->setActive(false);

        bench_mt(params);

        auto dlt1 = high_resolution_clock::now() - start;
        double delta1 = duration_cast<duration<double>>(dlt1).count();

        alog::logger().flush(3);
        alog::logger().waitingFlush();

        auto dlt2 = high_resolution_clock::now() - start;
        double delta2 = duration_cast<duration<double>>(dlt2).count();

        //if (alog::SaverPtr saver = alog::logger().findSaver("stdout"))
        //    saver->setActive(true);

        test_complete = true;
        t1.join();

        int cnt1 = int(params.howmany / delta1);
        int cnt2 = int(params.howmany / delta2);

        if (first_test) {
        log_info << log_format("Begin alloc mem   %? MB", begin_alloc_mem);
        first_test = false;
        }
        log_info << log_format("Elapsed (logging) %? secs; %?/sec", delta1, cnt1);
        log_info << log_format("Elapsed (flush)   %? secs; %?/sec", delta2, cnt2);

        uint32_t mem_max = max(mem_load) - start_mem;
        uint32_t mem_average = average(mem_load) - start_mem;
        log_info << log_format("Memory usage max: %? MB; average: %? MB", mem_max, mem_average);

        uint32_t cpu_max = max(cpu_load);
        uint32_t cpu_average = average(cpu_load);
        log_info << log_format("CPU usage    max: %? %; average: %? %", cpu_max, cpu_average);

        log_info << "---";

        delta1_times.push_back(delta1);
        delta2_times.push_back(delta2);

        count1.push_back(cnt1);
        count2.push_back(cnt2);

        sleep(3);
    }

    log_info << log_format("Average (logging) %? secs\t %?/sec", average(delta1_times), int(average(count1)));
    log_info << log_format("Average (flush)   %? secs\t %?/sec", average(delta2_times), int(average(count2)));

    log_info << "ALog test is stopped\n";
    alog::logger().flush();
    alog::logger().waitingFlush();
    alog::logger().stop();
}

#undef log_error_m
#undef log_warn_m
#undef log_info_m
#undef log_verbose_m
#undef log_debug_m
#undef log_debug2_m
