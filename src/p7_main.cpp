//
// Copyright(c) 2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "params_test.h"

void p7_test(const TestParams&, bool text_output);

int main(int /*argc*/, char** /*argv[]*/)
{
    TestParams params;
    p7_test(params, false);
    //p7_test(params, true);

    return 0;
}
