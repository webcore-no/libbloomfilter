#include "../src/bloomfilter.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



#define FCK_START(test_name) \
	START_TEST(test_name) { \
		bloomfilter_t *filter;\
		filter = bloomfilter_new(bloomfilter_shm_alloc);\
		ck_assert_ptr_nonnull(filter);

#define FCK_END \
	        bloomfilter_destroy(&filter, bloomfilter_shm_free); \
	} \
	END_TEST

#define FCK_SWAP_START(test_name) \
	START_TEST(test_name) { \
		bloomfilter_swap_t *filter;\
		filter = bloomfilterswap_new(bloomfilter_shm_alloc);\
		ck_assert_ptr_nonnull(filter);

#define FCK_SWAP_END \
	        bloomfilterswap_destroy(&filter, bloomfilter_shm_free); \
	} \
	END_TEST

#define STR(x) x, strlen(x)
#define PTR(x) &x, sizeof(x)

FCK_START(test_bloomfilter_create)
{}
FCK_END


FCK_START(test_bloomfilter_add)
{
    bloomfilter_add(filter, STR("foobar"));
    ck_assert_int_eq(1, bloomfilter_test(filter, STR("foobar")));
    ck_assert_int_eq(0, bloomfilter_test(filter, STR("barfoo")));
}
FCK_END

FCK_START(test_bloomfilter_add_multi)
{
    for(int i = 0; i < 1000000; i++) {
        bloomfilter_add(filter, PTR(i));
	ck_assert_int_eq(1, bloomfilter_test(filter, PTR(i)));
    }
}
FCK_END

FCK_START(test_bloomfilter_clear)
{
    for(int i = 0; i < 100000; i++) {
        bloomfilter_add(filter, PTR(i));
	ck_assert_int_eq(1, bloomfilter_test(filter, PTR(i)));
    }
    bloomfilter_clear(filter);
    for(int i = 0; i < 100000; i++) {
	ck_assert_int_eq(0, bloomfilter_test(filter, PTR(i)));
    }
}
FCK_END

// SWAP

FCK_SWAP_START(test_bloomfilterswap_create)
{}
FCK_SWAP_END


FCK_SWAP_START(test_bloomfilterswap_inital)
{
    // Check that everything returns true before swap
    ck_assert_int_eq(1, bloomfilterswap_test(filter, STR("foobar")));
}
FCK_SWAP_END

FCK_SWAP_START(test_bloomfilterswap_swap)
{
    bloomfilterswap_swap(filter);
    // Check that the swaped value contains nothing
    ck_assert_int_eq(0, bloomfilterswap_test(filter, STR("foobar")));
}
FCK_SWAP_END

FCK_SWAP_START(test_bloomfilterswap_add_and_swap)
{
    bloomfilterswap_add(filter, STR("foobar"));
    bloomfilterswap_swap(filter);
    // Check that the swaped value contains nothing
    ck_assert_int_eq(1, bloomfilterswap_test(filter, STR("foobar")));
}
FCK_SWAP_END

FCK_SWAP_START(test_bloomfilterswap_swap_and_add)
{
    bloomfilterswap_swap(filter);
    bloomfilterswap_add(filter, STR("foobar"));
    // Check that the swaped value contains nothing
    ck_assert_int_eq(1, bloomfilterswap_test(filter, STR("foobar")));
}
FCK_SWAP_END

Suite *bloomfilter_suite(void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_swap;

	s = suite_create("bloomfilter");

	/* Core test case */
	tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_bloomfilter_create);
	tcase_add_test(tc_core, test_bloomfilter_add);
	tcase_add_test(tc_core, test_bloomfilter_add_multi);
	tcase_add_test(tc_core, test_bloomfilter_clear);
	suite_add_tcase(s, tc_core);

	tc_swap = tcase_create("Swap");
	tcase_add_test(tc_swap, test_bloomfilterswap_create);
	tcase_add_test(tc_swap, test_bloomfilterswap_inital);
	tcase_add_test(tc_swap, test_bloomfilterswap_swap);
	tcase_add_test(tc_swap, test_bloomfilterswap_add_and_swap);
	tcase_add_test(tc_swap, test_bloomfilterswap_swap_and_add);
	suite_add_tcase(s, tc_swap);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = bloomfilter_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
