#include "hw_monitor.h"

#include "shared/break_point.h"
//#include "shared/logger/logger.h"
//#include "shared/qt/logger_operators.h"

#include <unistd.h>
#include <sstream>
#include <fstream>
#include <string>
#include <regex>
#include <cmath>

//#define log_error_m   alog::logger().error  (__FILE__, __func__, __LINE__, "Monitoring")
//#define log_warn_m    alog::logger().warn   (__FILE__, __func__, __LINE__, "Monitoring")
//#define log_info_m    alog::logger().info   (__FILE__, __func__, __LINE__, "Monitoring")
//#define log_verbose_m alog::logger().verbose(__FILE__, __func__, __LINE__, "Monitoring")
//#define log_debug_m   alog::logger().debug  (__FILE__, __func__, __LINE__, "Monitoring")
//#define log_debug2_m  alog::logger().debug2 (__FILE__, __func__, __LINE__, "Monitoring")

using namespace std;

uint32_t HwMonitor::procMem()
{
    string path = "/proc/" + to_string(getpid()) + "/status";
    ifstream file {path.c_str()};
    string line;
    while (getline(file, line))
    {
        if (memcmp(line.c_str(), "VmRSS", 5) == 0)
        {
            string result;
            try
            {
                static regex re {R"(VmRSS:\s+(\d+))"};
                smatch match;
                if (regex_search(line, match, re) && match.size() > 1)
                {
                    result = match.str(1);
                    return stoi(result) / 1024; // Size to MB
                }
            }
            catch (regex_error& e)
            {
                // Syntax error in the regular expression
            }
            return 0;
        }
    }
    return 0;
}

uint32_t HwMonitor::cpuLoad()
{
    _cpuStatusPrev = _cpuStatusNow;
    _cpuStatusNow  = readCpuStats();

    int size1 = _cpuStatusPrev.size();
    int size2 = _cpuStatusNow.size();

    if (!size1 || !size2 || size1 != size2)
        return 0;

    _cpuTotalTime = 1;

    for (int i = 0; i < size1; ++i)
        _cpuTotalTime += (_cpuStatusNow[i] - _cpuStatusPrev[i]);

    uint32_t result =
        100 - ((_cpuStatusNow[size2 - 1] - _cpuStatusPrev[size1 - 1]) * 100 / _cpuTotalTime);

    return result;
}

uint32_t HwMonitor::procLoad()
{
    _pstatPrev = _pstatNow;

    string path {"/proc/" + to_string(getpid()) + "/stat"};
    FILE* dataProc = fopen(path.c_str(), "r");
    if (dataProc == nullptr)
    {
        //log_debug2_m << "No info about process stat: " << proc.pid;
        return 0;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

    fscanf(dataProc, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u "
                     "%lu %lu %ld %ld",
           &_pstatNow.utime_ticks, &_pstatNow.stime_ticks,
           &_pstatNow.cutime_ticks, &_pstatNow.cstime_ticks);

#pragma GCC diagnostic pop

    fclose(dataProc);

    uint32_t result =
        round(100.0 * (((_pstatNow.utime_ticks + _pstatNow.stime_ticks)
                                 - (_pstatPrev.utime_ticks + _pstatPrev.stime_ticks))
              / double(_cpuTotalTime)));

    return result;
}

vector<float> HwMonitor::readCpuStats()
{
    // Открываем данные о процессорном времени в целом:
    vector<float> ret;
    ifstream file {"/proc/stat"};

    // Процессорное время в целом:
    string dummy;
    file >> dummy; (void) dummy;
    for (int i = 0; i < 4; ++i)
    {
        int val;
        file >> val;
        ret.push_back(val);
    }

//    if (ret[1] != 0)
//        break_point

//    QFile dataCPU {"/proc/stat"};
//    if (!dataCPU.open(QFile::ReadOnly | QFile::Text))
//        return ret;

//    //QTextStream cpuStatFile(&dataCPU);

//    // Процессорное время в целом:
//    QString dummy;
//    cpuStatFile >> dummy; (void) dummy;
//    for (int i = 0; i < 4; ++i)
//    {
//        int val;
//        cpuStatFile >> val;
//        ret.push_back(val);
//    }
//    dataCPU.close();

    return ret;
}
