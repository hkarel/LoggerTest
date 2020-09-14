//
// Copyright(c) 2015 Gabi Melman.
// Modified     2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "params_test.h"

void g3log_test(const TestParams& params);

int main(int /*argc*/, char** /*argv[]*/)
{
    TestParams params;
    g3log_test(params);

    return 0;
}
