#include "ds/item.h"
#include "minunit.h"

void test_setup() {}
void test_teardown() {}

MU_TEST(test_item_init) {
    item *itp = item_init();
    mu_assert(itp != NULL, "Heap allocation of item creation failed");
    item_free(itp);
}

MU_TEST_SUITE(item_test_suite) {
    MU_SUITE_CONFIGURE(test_setup, test_teardown);

    MU_RUN_TEST(test_item_init);
}

MU_MAIN(MU_RUN_SUITE(item_test_suite); MU_REPORT(); return MU_EXIT_CODE;)
