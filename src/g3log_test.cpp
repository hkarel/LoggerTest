/** ==========================================================================
* 2011 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
*
* For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#include "params_test.h"
#include "hw_monitor.h"

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

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

static void thread_func(int howmany)
{
    //pid_t tid = (syscall(SYS_gettid));
    for (int i = 0; i < howmany; ++i)
    {
        //logger->trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
        //spdlog::trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
        LOG(G3LOG_DEBUG) << "Hello logger: msg number " << i;
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
            threads.push_back(std::thread(thread_func, msgs_per_thread + msgs_per_thread_mod));
        else
            threads.push_back(std::thread(thread_func, msgs_per_thread));
    }

    for (auto& t : threads)
        t.join();
}

void g3log_test(const TestParams& params)
{
    using namespace g3;
    using std::chrono::high_resolution_clock;

    HwMonitor hwmon;
    uint32_t start_mem = hwmon.procMem();

    std::cout << "G3log speed test is running\n";
    std::cout << "-------------------------------------------------\n";
    std::cout << "Messages     : " << params.howmany << std::endl;
    std::cout << "Threads      : " << params.threads << std::endl;
    std::cout << "Total iters  : " << params.iters << std::endl;
    std::cout << "Start memory : " << start_mem << " MB\n";
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

        std::unique_ptr<LogWorker> logworker {LogWorker::createLogWorker()};
        auto sinkHandle = logworker->addSink(std::make_unique<FileSink>("G3log", params.log_path),
                                             &FileSink::fileWrite);
        initializeLogging(logworker.get());

//            auto worker = g3::LogWorker::createLogWorker();
//            auto handle = worker->addDefaultLogger("G3log", params.log_path);
//            g3::initializeLogging(worker.get());
        //std::future<std::string> log_file_name = handle->call(&g3::FileSink::fileName);

//            // Exmple of overriding the default formatting of log entry
//            auto changeFormatting = handle->call(&g3::FileSink::overrideLogDetails, g3::LogMessage::FullLogDetailsToString);
//            //const std::string newHeader = "\t\tLOG format: [YYYY.MM.DD hh:mm:ss uuu* LEVEL THREAD_ID] message\n\t\t(uuu*: microseconds fractions of the seconds value)\n\n";
//            const std::string newHeader = "\t\tLOG format: [YYYY.MM.DD hh:mm:ss uuu* LEVEL THREAD_ID] message\n";
//            // example of ovrriding the default formatting of header
//            auto changeHeader = handle->call(&g3::FileSink::overrideLogHeader, newHeader);

//            changeFormatting.wait();
//            changeHeader.wait();

        auto dlt0 = high_resolution_clock::now() - start0;
        double delta0 = duration_cast<duration<double>>(dlt0).count();

        uint32_t begin_alloc_mem = hwmon.procMem() - start_mem;

        auto start = high_resolution_clock::now();

        bench_mt(params);

        auto dlt1 = high_resolution_clock::now() - start;
        double delta1 = duration_cast<duration<double>>(dlt1).count();

        //logger->flush();

        logworker.reset();

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

    std::cout << "G3log test is stopped\n\n";
}
