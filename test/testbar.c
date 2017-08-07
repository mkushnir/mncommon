#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif


static int
gsetup(void **state)
{
    (void)state;
    print_message("%s\n", __func__);
    return 0;
}


static int
gteardown(void **state)
{
    (void)state;
    print_message("%s\n", __func__);
    return 0;
}


static int
test1_setup(void **state)
{
    (void)state;
    print_message("%s\n", __func__);
    return 0;
}


static void
test1(void **state)
{
    (void)state;
    print_message("%s\n", __func__);
}


static int
test1_teardown(void **state)
{
    (void)state;
    print_message("%s\n", __func__);
    return 0;
}


int
main(void)
{
    int res;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test1, test1_setup, test1_teardown),
    };

    cmocka_set_message_output(CM_OUTPUT_STDOUT); //default
    res = cmocka_run_group_tests(tests, gsetup, gteardown);
    return 0;
}
