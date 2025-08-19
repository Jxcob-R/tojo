#include "ds/item.h"
#include "minunit.h"

/*
 * Use standard fail messages:
 * These should be general enough such that they do not need significant
 * modification if behaviour is to change in future versions.
 */

/* Array of items is not allocated */
static const char *arr_alloc_fail_msg = "Array was not allocated";
/* A single item is not allocated on the heap */
static const char *item_alloc_fail_msg = "Item was not heap allocated";

void test_setup() {}
void test_teardown() {}

MU_TEST(test_item_init) {
    item *itp = item_init();
    mu_assert(itp != NULL, item_alloc_fail_msg);
    item_free(itp);
}

MU_TEST(test_item_array_init_free) {
    item **item_arr = item_array_init(5);
    mu_assert(item_arr != NULL, arr_alloc_fail_msg);
    for (int i = 0; i < 5; i++) {
        mu_assert(item_arr[i] != NULL, item_alloc_fail_msg);
    }

    mu_assert(item_arr[5] == NULL, "Item array does not end with a NULL");

    item_array_free(&item_arr, 5);
    mu_assert(item_arr == NULL,
              "Item array array pointer left hanging (not NULL)");
}

MU_TEST(test_item_array_init_none) {
    item **item_arr = item_array_init(0);
    mu_assert(item_arr != NULL, arr_alloc_fail_msg);

    mu_assert(item_arr[0] == NULL, "Item array does not contain just NULL");

    item_array_free(&item_arr, 0);
    mu_assert(item_arr == NULL,
              "Item array array pointer left hanging (not NULL)");
}

MU_TEST(test_item_array_init_empty) {
    item **item_arr_empty = item_array_init_empty(3);
    mu_assert(item_arr_empty != NULL, arr_alloc_fail_msg);
    /* Check last 'terminating' position as well */
    for (int i = 0; i <= 3; i++) {
        mu_assert(item_arr_empty[i] == NULL, "Incorrect non-NULL value");
    }
    item_array_free(&item_arr_empty, 3);
    mu_assert(item_arr_empty == NULL,
              "Item array array pointer left hanging (not NULL)");
}

MU_TEST(test_item_array_resize) {
    item **item_arr_to_resize = item_array_init(16);

    item **new_array = item_array_resize(item_arr_to_resize, 32);
    mu_assert(new_array != NULL, arr_alloc_fail_msg);
    mu_assert(new_array[32] == NULL, "Last index is not NULL or unreadable");

    /* This should have no leaks */
    item_array_free(&new_array, 32);
}

MU_TEST_SUITE(item_test_suite) {
    MU_SUITE_CONFIGURE(test_setup, test_teardown);

    MU_RUN_TEST(test_item_init);
    MU_RUN_TEST(test_item_array_init_free);
    MU_RUN_TEST(test_item_array_init_none);
    MU_RUN_TEST(test_item_array_init_empty);
    MU_RUN_TEST(test_item_array_resize);
}

MU_MAIN(MU_RUN_SUITE(item_test_suite); MU_REPORT(); return MU_EXIT_CODE;)
