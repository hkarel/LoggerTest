//
// Copyright(c) 2020 Pavel Karelin (hkarel), <hkarel@yandex.ru>
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "params_test.h"

void alog_test(const TestParams&);

int main(int /*argc*/, char** /*argv[]*/)
{
    TestParams params;
    //params.threads = 1;

    alog_test(params);

    return 0;
}

#undef log_error_m
#undef log_warn_m
#undef log_info_m
#undef log_verbose_m
#undef log_debug_m
#undef log_debug2_m
