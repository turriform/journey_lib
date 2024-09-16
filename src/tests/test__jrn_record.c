#include "jrn_internal.h"
#include "jrn_test_set.h"

void test_record_basics(void)
{
    jrn_init("test", "log", LOG_LEVEL_ERROR, LOG_FILE_JSON, true);

   
    jrn_destroy();
}

void run_record_tests(void)
{
    test_record_basics();
}