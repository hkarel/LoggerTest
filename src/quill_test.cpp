
#include "params_test.h"
#include "hw_monitor.h"

#include "quill/Quill.h"
#include "quill/Utility.h"
#include "shared/utils.h"

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



struct UserTestQ
{
    int val1;
    double val2;
    std::string str100;

    UserTestQ() {
        str100 = "1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890";
    }

    QUILL_COPY_LOGGABLE; /** Tagged as safe to copy **/
};

std::ostream& operator<< (std::ostream& os, UserTestQ const& u)
{
    os << "Value1: " << u.val1
       << " Value2: " << u.val2
       << " String100: " << u.str100;
    return os;
}

static void thread_func(quill::Logger* logger, int howmany, const TestParams& params)
{
    quill::preallocate();
    for (int i = 0; i < howmany; ++i)
    {
        if (params.user_test)
        {
            UserTestQ u;
            u.val1 = i;
            u.val2 = i;
            if (params.message_work_thread)
                LOG_TRACE_L3(logger, u8"Hello logger. {}" , quill::utility::to_string(u));
            else
                LOG_TRACE_L3(logger, u8"Hello logger. {}" , u);
        }
        else
        {
            LOG_TRACE_L3(logger, u8"Hello logger: msg number {}", i);
        }
    }
}

static void bench_mt(quill::Logger* logger, const TestParams& params)
{
    vector<thread> threads;
    int msgs_per_thread = params.howmany / params.threads;
    int msgs_per_thread_mod = params.howmany % params.threads;

    for (int t = 0; t < (params.threads - 1); ++t)
        threads.push_back(std::thread(thread_func, logger, msgs_per_thread, params));

    if (msgs_per_thread_mod)
        thread_func(logger, msgs_per_thread + msgs_per_thread_mod, params);
    else
        thread_func(logger, msgs_per_thread, params);

    for (auto& t : threads)
        t.join();
}

void quill_test(const TestParams& params)
{
    using std::chrono::high_resolution_clock;

    try
    {
        // Попытка ускорить рабочий поток. Дает прирост ~0.15 сек при среднем времени
        // сохранения тестового файла 6.2 сек.
        // quill::config::set_backend_thread_sleep_duration(std::chrono::nanoseconds{0});

        // set backend on it's own cpu
        // quill::config::set_backend_thread_cpu_affinity(1);

        // Start the logging backend thread
        quill::start();

        std::vector<quill::Logger*> loggers;
        for (int i = 0; i < params.iters; ++i)
        {
            std::string quill_file = params.log_path + "/quill-async" + utl::toString(i) + ".log";
            quill::Handler* file_handler = quill::file_handler(quill_file, "w");

            std::string quill_logger = "logger_" + utl::toString(i);
            quill::Logger* logger = quill::create_logger(quill_logger.data(), file_handler);

            loggers.push_back(logger);
        }

        HwMonitor hwmon;
        uint32_t start_mem = hwmon.procMem();

        std::cout << "Quill speed test is running\n";
        std::cout << "-------------------------------------------------\n";
        std::cout << "Messages      : " << params.howmany << std::endl;
        std::cout << "Threads       : " << params.threads << std::endl;
        std::cout << "Total iters   : " << params.iters << std::endl;
        std::cout << "Start memory  : " << start_mem << " MB\n";
        std::cout << "-------------------------------------------------\n";
        std::cout.flush();

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

            quill::Logger* logger = loggers[i];

            //logger->init_backtrace(2, quill::LogLevel::Error);
            logger->set_log_level(quill::LogLevel::TraceL3);

            auto dlt0 = high_resolution_clock::now() - start0;
            double delta0 = duration_cast<duration<double>>(dlt0).count();

            uint32_t begin_alloc_mem = hwmon.procMem() - start_mem;

            auto start = high_resolution_clock::now();

            bench_mt(logger, params);

            auto dlt1 = high_resolution_clock::now() - start;
            double delta1 = duration_cast<duration<double>>(dlt1).count();

            //logger->flush_backtrace();
            quill::flush();

            auto dlt2 = high_resolution_clock::now() - start;
            double delta2 = duration_cast<duration<double>>(dlt2).count();

            test_complete = true;
            t1.join();

            int cnt1 = int(params.howmany / delta1);
            int cnt2 = int(params.howmany / delta2);

            if (first_test) {
            std::cout << "Begin alloc mem   " << begin_alloc_mem << " MB\n";
            first_test = false;
            }
            std::cout << "Elapsed (create)  " << delta0 << " secs\n";
            std::cout << "Elapsed (logging) " << delta1 << " secs\t " << cnt1 << "/sen\n";
            std::cout << "Elapsed (flush)   " << delta2 << " secs\t " << cnt2 << "/sec\n";

            uint32_t mem_max = max(mem_load) - start_mem;
            uint32_t mem_average = average(mem_load) - start_mem;
            std::cout << "Memory usage max: " << mem_max << " MB; average: " << mem_average << " MB\n";

            uint32_t cpu_max = max(cpu_load);
            uint32_t cpu_average = average(cpu_load);
            std::cout << "CPU usage    max: " << cpu_max << " %; average: " << cpu_average << " %\n";

            std::cout << "---\n";
            std::cout.flush();

            delta1_times.push_back(delta1);
            delta2_times.push_back(delta2);

            count1.push_back(cnt1);
            count2.push_back(cnt2);

            sleep(3);
        }

        std::cout << "Average (logging) " << average(delta1_times) << " secs\t "<< int(average(count1)) << "/sec\n";
        std::cout << "Average (flush)   " << average(delta2_times) << " secs\t "<< int(average(count2)) << "/sec\n";

        std::cout << "Quill test is stopped\n\n";
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
    }
}
