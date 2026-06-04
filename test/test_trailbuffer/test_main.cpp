// Claude Generated: version 1 - Unity tests for per-PRN satellite trail ring buffer
#include <unity.h>
#include "store/TrailBuffer.h"

void setUp() {}
void tearDown() {}

void test_appends_and_preserves_order() {
    TrailBuffer t;
    t.append(5, {72, 214, 42, true,  0});
    t.append(5, {73, 215, 41, true, 60});
    TEST_ASSERT_EQUAL_INT(2, (int)t.samples(5).size());
    TEST_ASSERT_EQUAL_INT( 0, (int)t.samples(5)[0].tsSec);
    TEST_ASSERT_EQUAL_INT(60, (int)t.samples(5)[1].tsSec);
}

void test_empty_prn_returns_empty() {
    TrailBuffer t;
    TEST_ASSERT_EQUAL_INT(0, (int)t.samples(99).size());
}

void test_evicts_older_than_window() {
    TrailBuffer t;
    t.append(5, {72, 214, 42, true,    0});
    t.append(5, {73, 215, 41, true, 1000});
    t.evictOlderThan(1000, 900);  // window=900s: entry at t=0 is 1000s old, evicted
    TEST_ASSERT_EQUAL_INT(1, (int)t.samples(5).size());
    TEST_ASSERT_EQUAL_INT(1000, (int)t.samples(5)[0].tsSec);
}

void test_evict_keeps_entries_within_window() {
    TrailBuffer t;
    t.append(5, {72, 214, 42, true, 200});
    t.append(5, {73, 215, 41, true, 500});
    t.evictOlderThan(500, 900);  // both are within 900s of t=500
    TEST_ASSERT_EQUAL_INT(2, (int)t.samples(5).size());
}

void test_retain_only_drops_absent_prns() {
    TrailBuffer t;
    t.append(5, {72, 214, 42, true, 0});
    t.append(9, {10,  40, 18, false, 0});
    t.retainOnly({5});
    TEST_ASSERT_EQUAL_INT(1, (int)t.samples(5).size());
    TEST_ASSERT_EQUAL_INT(0, (int)t.samples(9).size());
}

void test_retain_only_empty_set_clears_all() {
    TrailBuffer t;
    t.append(5, {72, 214, 42, true, 0});
    t.retainOnly({});
    TEST_ASSERT_EQUAL_INT(0, (int)t.samples(5).size());
}

void test_prns_lists_tracked_prns() {
    TrailBuffer t;
    t.append(5,  {72, 214, 42, true, 0});
    t.append(12, {30,  90, 25, true, 0});
    auto prns = t.prns();
    TEST_ASSERT_EQUAL_INT(2, (int)prns.size());
}

void test_ring_cap_evicts_oldest() {
    TrailBuffer t;
    for (int i = 0; i < 22; ++i) {
        t.append(5, {45, 180, 30, true, (uint32_t)(i * 60)});
    }
    // kMaxPerPrn is 20; first two entries should be gone
    TEST_ASSERT_EQUAL_INT(20, (int)t.samples(5).size());
    TEST_ASSERT_EQUAL_INT(2 * 60, (int)t.samples(5)[0].tsSec);  // oldest kept = index 2
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_appends_and_preserves_order);
    RUN_TEST(test_empty_prn_returns_empty);
    RUN_TEST(test_evicts_older_than_window);
    RUN_TEST(test_evict_keeps_entries_within_window);
    RUN_TEST(test_retain_only_drops_absent_prns);
    RUN_TEST(test_retain_only_empty_set_clears_all);
    RUN_TEST(test_prns_lists_tracked_prns);
    RUN_TEST(test_ring_cap_evicts_oldest);
    return UNITY_END();
}
