//
// Copyright(c) 2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "params_test.h"

void spdlog_test(const TestParams&);

int main(int /*argc*/, char** /*argv[]*/)
{
    TestParams params;
    spdlog_test(params);

    return 0;
}
