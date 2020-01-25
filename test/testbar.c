#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <mncommon/dumpm.h>
#include <mncommon/util.h>

#include <mncommon/array.h>


#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

#ifdef MNUNIT_RUN_FAILURES
#define MNUNIT_MARK_FAIL
#else
#define MNUNIT_MARK_FAIL skip()
#endif

static int
gsetup(void **state)
{
    (void)state;
    //print_message("%s\n", __func__);
    return 0;
}


static int
gteardown(void **state)
{
    (void)state;
    //print_message("%s\n", __func__);
    return 0;
}


static int
test1_setup(void **state)
{
    (void)state;
    //print_message("%s\n", __func__);
    return 0;
}


static void
test1(void **state)
{
    (void)state;
    //print_message("%s\n", __func__);
}


static int
test1_teardown(void **state)
{
    (void)state;
    //print_message("%s\n", __func__);
    return 0;
}


static void
test_array_init_zero_elsz(UNUSED void **state)
{
    expect_assert_failure(array_init(NULL, 0, 0, NULL, NULL));
}


static void
test_array_init_null_pointer(UNUSED void **state)
{
    MNUNIT_MARK_FAIL;
    array_init(NULL, 1, 0, NULL, NULL);
}


static void
test_array_init_illegal_pointer(UNUSED void **state)
{
    MNUNIT_MARK_FAIL;
    array_init((mnarray_t *)0xffffffffffffffff, 1, 0, NULL, NULL);
}


static void
test_array_init_illegal_callback(UNUSED void **state)
{
    mnarray_t a;
    MNUNIT_MARK_FAIL;
    assert_int_equal(array_init(&a, 1, 1, (array_initializer_t)0x1, NULL), 0);
    assert_int_equal(array_fini(&a), 0);
}


static void
test_array_init_ok0(UNUSED void **state)
{
    mnarray_t a;
    assert_int_equal(array_init(&a, 1, 0, NULL, NULL), 0);
    assert_int_equal(array_fini(&a), 0);
}


static void
test_array_init_ok1(UNUSED void **state)
{
    mnarray_t a;
    assert_int_equal(array_init(&a, 1, 1, NULL, NULL), 0);
    assert_int_equal(array_fini(&a), 0);
}


static int
myinit(UNUSED void *u)
{
    /* post */
    check_expected(u); //expect_not_memory
    check_expected(u); //expect_any
    function_called();

    return 0;
}


static int
myfini(UNUSED void *u)
{
    /* post */
    check_expected(u); //expect_not_memory
    check_expected(u); //expect_any
    function_called();

    return 0;
}


static void
test_array_init_ok2(UNUSED void **state)
{
    mnarray_t a;

    /* pre */

    expect_not_memory(myinit, u, &a, 1);
    expect_any(myinit, u);
    expect_function_call(myinit);

    expect_not_memory(myfini, u, &a, 1);
    expect_any(myfini, u);
    expect_function_call(myfini);

    assert_int_equal(array_init(&a, 1, 1, myinit, myfini), 0);
    assert_int_equal(array_fini(&a), 0);
}


int
main(void)
{
    int res;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test1, test1_setup, test1_teardown),
        cmocka_unit_test(test_array_init_zero_elsz),
        cmocka_unit_test(test_array_init_null_pointer),
        cmocka_unit_test(test_array_init_illegal_pointer),
        cmocka_unit_test(test_array_init_illegal_callback),
        cmocka_unit_test(test_array_init_ok0),
        cmocka_unit_test(test_array_init_ok1),
        cmocka_unit_test(test_array_init_ok2),
    };

    cmocka_set_message_output(CM_OUTPUT_TAP); //default
    res = cmocka_run_group_tests(tests, gsetup, gteardown);
    return 0;
}
