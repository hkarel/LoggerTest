//
// Copyright(c) 2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <sys/types.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

class HwMonitor
{
public:
    uint32_t cpuLoad();
    uint32_t procLoad();
    uint32_t procMem();

private:
    struct PStat
    {
        ulong utime_ticks  = {0};
        long  cutime_ticks = {0};
        ulong stime_ticks  = {0};
        long  cstime_ticks = {0};
    };

    std::vector<float> readCpuStats();

private:
    std::vector<float> _cpuStatusPrev;
    std::vector<float> _cpuStatusNow;

    PStat    _pstatPrev;
    PStat    _pstatNow;
    uint32_t _cpuTotalTime = {0};
};
