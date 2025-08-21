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

MU_TEST(test_item_array_find) {
    item **item_arr = item_array_init(6);
    item_arr[3]->item_id = 100;
    mu_assert_int_eq(3, item_array_find((const item *const *)item_arr, 100));
    item_array_free(&item_arr, 6);
}

MU_TEST(test_item_array_find_in_empty) {
    item **item_arr = item_array_init_empty(9);
    item *item_to_find = item_init();
    item_arr[3] = item_to_find;
    item_to_find->item_id = 100;
    /* This fails since NULL is found earlier */
    mu_assert(SIZE_MAX == item_array_find((const item *const *)item_arr, 100),
              "Item was somehow found -- this is undefined");
    /* Clear it as if it never existed */
    item_free(item_to_find);
    item_arr[3] = NULL;
    item_array_free(&item_arr, 9);
}

MU_TEST(test_item_count_items) {
    item **item_arr = item_array_init(22);
    mu_assert(item_count_items(item_arr) == 22,
              "Incorrect number of items listed");
    /* Clear it as if it never existed */
    item_array_free(&item_arr, 22);
}

MU_TEST(test_item_count_items_none) {
    item **item_arr = item_array_init_empty(10);
    mu_assert(item_count_items(item_arr) == 0, "Some numbers were counted");
    /* Clear it as if it never existed */
    item_array_free(&item_arr, 10);
}

MU_TEST(test_item_array_add) {
    item **item_arr_src_0 = item_array_init(4);
    item **item_arr_src_1 = item_array_init(4);
    item **item_arr_dest = item_array_init_empty(8);

    item_arr_src_0[2]->item_id = 5;
    item_arr_src_1[1]->item_id = 7;

    item_array_add(item_arr_dest, &item_arr_src_0, 4);
    item_array_add(item_arr_dest + 4, &item_arr_src_1, 4);

    mu_assert(item_arr_dest[2]->item_id == 5, "Incorrect ID in added position");
    mu_assert(item_arr_dest[4 + 1]->item_id == 7,
              "Incorrect ID in added position");

    item_array_free(&item_arr_dest, 8);
}

MU_TEST(test_item_set_name) {
    item *itp = item_init();

    /* Initial */
    strcpy(itp->item_name, "Initial string");
    mu_assert_string_eq("Initial string", itp->item_name);

    /* Change */
    char *name = malloc(sizeof("This is some name"));
    memcpy(name, "This is some name", sizeof("This is some name"));

    item_set_name(itp, &name);
    mu_assert_string_eq("This is some name", itp->item_name);

    item_free(itp);
}

MU_TEST(test_item_set_name_deep) {
    item *itp = item_init();
    const char *name = "Deeply set name";
    item_set_name_deep(itp, name, strlen(name));
    mu_assert_string_eq(name, itp->item_name);
    item_free(itp);
}

MU_TEST(test_item_set_code) {
    /* This is also a test that the code generation produces acceptably entropic
    * codes based on 'similar' IDs */
    item *itp = item_init();
    itp->item_id = 0;
    item_set_code(itp);
    mu_assert_string_n_eq("aaaaaaa", itp->item_code, 7);
    itp->item_id++;
    item_set_code(itp);
    mu_assert(itp->item_code[0] != 'a', "Code has not substantially changed");
    item_free(itp);
}

MU_TEST_SUITE(item_test_suite) {
    MU_SUITE_CONFIGURE(test_setup, test_teardown);

    MU_RUN_TEST(test_item_init);
    MU_RUN_TEST(test_item_array_init_free);
    MU_RUN_TEST(test_item_array_init_none);
    MU_RUN_TEST(test_item_array_init_empty);
    MU_RUN_TEST(test_item_array_resize);
    MU_RUN_TEST(test_item_array_find);
    MU_RUN_TEST(test_item_array_find_in_empty);
    MU_RUN_TEST(test_item_count_items);
    MU_RUN_TEST(test_item_count_items_none);
    MU_RUN_TEST(test_item_array_add);
    MU_RUN_TEST(test_item_set_name);
    MU_RUN_TEST(test_item_set_name_deep);
    MU_RUN_TEST(test_item_set_code);
}

MU_MAIN(MU_RUN_SUITE(item_test_suite); MU_REPORT(); return MU_EXIT_CODE;)
