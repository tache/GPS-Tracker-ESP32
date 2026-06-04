// Claude Generated: version 1 - Unity tests for polarPoint projection
#include <unity.h>
#include "view/Projection.h"

void setUp() {}
void tearDown() {}

void test_zenith_maps_to_center() {
    auto p = projection::polarPoint(90, 123, 120, 120, 108);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 120.0, p.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 120.0, p.y);
}

void test_north_horizon_is_top() {
    auto p = projection::polarPoint(0, 0, 120, 120, 108);  // el=0 az=0 (N)
    TEST_ASSERT_FLOAT_WITHIN(0.01, 120.0, p.x);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 12.0, p.y);  // cy - radius = 120 - 108 = 12
}

void test_east_horizon_is_right() {
    auto p = projection::polarPoint(0, 90, 120, 120, 108);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 228.0, p.x);  // cx + radius = 120 + 108 = 228
    TEST_ASSERT_FLOAT_WITHIN(0.5, 120.0, p.y);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_zenith_maps_to_center);
    RUN_TEST(test_north_horizon_is_top);
    RUN_TEST(test_east_horizon_is_right);
    return UNITY_END();
}
