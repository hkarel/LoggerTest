#pragma once

#include <string>
#include <numeric>
#include <algorithm>

struct TestParams
{
    //--- Common params ---
    //int howmany  = 1000000;
    int howmany  = 5000000;
    int threads  = 4;
    int iters    = 5;

    // Disk system (SSD/HDD)
    std::string log_path = "/tmp";  // SSD
    //std::string log_path = "/mnt/storage/Downloads";  // HDD

    //--- ALog params ---
    // Use formatting mode for log-messages
    bool format_func = true;
    std::string alog_file = {log_path + "/alog-async.log"};

    //--- P7 Log params ---
    //int pool_size = 1024; // 1MB memory for buffers
    int pool_size = 1024*1024;
    std::string p7bin_file = {log_path + "/p7bin-async.log"};
    std::string p7txt_file = {log_path + "/p7txt-async.log"};

    //--- SpdLog params ---
    //int queue_size = {std::min(howmany + 2, 500000)};
    int queue_size = {std::min(howmany + 2, 3000000)};
    std::string spdlog_file = {log_path + "/spdlog-async.log"};
};

template<typename T>
typename T::value_type sum(const T& elements)
{
    return std::accumulate(elements.begin(), elements.end(), typename T::value_type(0));
}

template<typename T>
typename T::value_type max(const T& elements)
{
    auto it = std::max_element(elements.begin(), elements.end());
    return (it != elements.end()) ? *it : 0;
}

template<typename T>
double average(const T& elements)
{
    double s = sum(elements);
    return (s / elements.size());
}
