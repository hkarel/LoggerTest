#include "params_test.h"
#include "hw_monitor.h"

#include "P7_Trace.h"
#include "P7_Telemetry.h"

#include <unistd.h>
#include <sys/syscall.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <thread>
#include <numeric>
#include <algorithm>

using namespace std;
using namespace std::chrono;

static void thread_func(IP7_Trace* l_pTrace, IP7_Trace::hModule l_hModule, int howmany)
{
    //pid_t tid = (syscall(SYS_gettid));
    for (int i = 0; i < howmany; ++i)
    {
        l_pTrace->P7_QTRACE(0, l_hModule, TM("Test trace message #%u"), i);
        //logger->trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
        //spdlog::trace(u8"LWP{} [{}:{} LoggerTest] Hello logger: msg number {}", tid, "spdlog_test.cpp", __LINE__, i);
    }
}

static void bench_mt(IP7_Trace* l_pTrace, IP7_Trace::hModule l_hModule, const TestParams& params)
{
    vector<thread> threads;
    int msgs_per_thread = params.howmany / params.threads;
    int msgs_per_thread_mod = params.howmany % params.threads;

    for (int t = 0; t < (params.threads - 1); ++t)
        threads.push_back(std::thread(thread_func, l_pTrace, l_hModule, msgs_per_thread));

    if (msgs_per_thread_mod)
        thread_func(l_pTrace, l_hModule, msgs_per_thread + msgs_per_thread_mod);
    else
        thread_func(l_pTrace, l_hModule, msgs_per_thread);

    for (auto& t : threads)
        t.join();
}

void p7_test(const TestParams& params, bool text_output)
{
    using std::chrono::high_resolution_clock;

    HwMonitor hwmon;
    uint32_t start_mem = hwmon.procMem();

    stringstream ss;
    if (text_output)
        ss << " /P7.Sink=FileTxt";
    else
        ss << " /P7.Sink=FileBin";

    ss << " /P7.Dir=" << params.log_path;
    ss << " /P7.Pool=" << params.pool_size;
    string sArgs = ss.str();

    std::cout << "P7 speed test is running\n";
    std::cout << "-------------------------------------------------\n";
    std::cout << "Messages      : " << params.howmany << std::endl;
    std::cout << "Threads       : " << params.threads << std::endl;
    std::cout << "Total iters   : " << params.iters << std::endl;
    std::cout << "Pool size     : " << params.pool_size / 1024 << " MB\n";
    std::cout << "Start memory  : " << start_mem << " MB\n";
    std::cout << "Output        : " << (text_output ? "text\n" : "binary\n");
    std::cout << "Log arguments : " << sArgs << "\n";
    std::cout << "-------------------------------------------------\n";
    std::cout.flush();

    std::vector<double> delta1_times;
    std::vector<double> delta2_times;

    std::vector<int> count1;
    std::vector<int> count2;

    IP7_Client        *l_pClient = 0;
    IP7_Trace         *l_pTrace  = 0;
    IP7_Trace::hModule l_hModule = 0;

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

        high_resolution_clock::time_point start;
        double delta0, delta1, delta2;
        uint32_t begin_alloc_mem;

        auto start0 = high_resolution_clock::now();

        //create P7 client object -> Binary sync, 1MB memory for buffers (/P7.Pool=1024)
        l_pClient = P7_Create_Client(sArgs.c_str());

        if (l_pClient)
        {
            //create P7 trace object 1
            l_pTrace = P7_Create_Trace(l_pClient, TM("Trace channel 1"));
            if (l_pTrace)
            {
                l_pTrace->Register_Thread(TM("Application"), 0);
                l_pTrace->Register_Module(TM("Main"), &l_hModule);

                auto dlt0 = high_resolution_clock::now() - start0;
                delta0 = duration_cast<duration<double>>(dlt0).count();

                begin_alloc_mem = hwmon.procMem() - start_mem;

                start = high_resolution_clock::now();

                bench_mt(l_pTrace, l_hModule, params);

                auto dlt1 = high_resolution_clock::now() - start;
                delta1 = duration_cast<duration<double>>(dlt1).count();

                l_pTrace->Unregister_Thread(0);
                l_pTrace->Release();
            }

            l_pClient->Release();
        }

        auto dlt2 = high_resolution_clock::now() - start;
        delta2 = duration_cast<duration<double>>(dlt2).count();

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

    std::cout << "P7 test is stopped\n\n";
}
