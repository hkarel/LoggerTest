//
// Copyright(c) 2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "params_test.h"

void alog_test(const TestParams& params);
void spdlog_test(const TestParams& params);

int main(int /*argc*/, char** /*argv[]*/)
{
    TestParams params;

    spdlog_test(params);
    alog_test(params);

    return 0;
}
