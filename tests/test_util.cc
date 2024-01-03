// #define NDEBUG  // 这个宏必须定义在#include之前有效，可以取消assert;

#include "lyslg.h"
#include <assert.h>

lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

void test_assert() {
    LYSLG_LOG_INFO(g_logger) << lyslg::BacktraceToString(10,2,"    ");
    // LYSLG_ASSERT(false);
    LYSLG_ASSERT2(1 == 0,"adf");
}


int main(int argc,char** argv) {
    test_assert();

    return 0;
}
