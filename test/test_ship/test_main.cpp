// test/test_ship/test_main.cpp
// Claude Generated: version 1 - Unity tests for ShipSteering pure math
#include <unity.h>
#include "view/ShipSteering.h"
#include <cmath>
using namespace ship_math;

void setUp() {}
void tearDown() {}

// --- wrapAngle ---

void test_wrapAngle_positive_overflow() {
    float result = wrapAngle(float(M_PI) + 0.1f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -(float(M_PI) - 0.1f), result);
}

void test_wrapAngle_negative_overflow() {
    float result = wrapAngle(-(float(M_PI) + 0.1f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, float(M_PI) - 0.1f, result);
}

void test_wrapAngle_zero_unchanged() {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, wrapAngle(0.0f));
}

// --- stepHeading ---

void test_stepHeading_within_range_goes_direct() {
    float h = stepHeading(0.0f, 0.1f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, h);
}

void test_stepHeading_beyond_range_clamps() {
    float desired = float(M_PI) / 2.0f;
    float result  = stepHeading(0.0f, desired);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, kMaxTurnRad, result);
}

void test_stepHeading_negative_turn_clamps() {
    float desired = -float(M_PI) / 2.0f;
    float result  = stepHeading(0.0f, desired);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -kMaxTurnRad, result);
}

// --- computeDesiredHeading ---

void test_desired_heading_no_satellites_points_to_exit() {
    ShipState s;
    s.pos     = {120.0f, 120.0f};
    s.heading = 0.0f;
    s.exitPt  = {220.0f, 120.0f};
    std::vector<Point2f> empty;
    float h = computeDesiredHeading(s, empty);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, h);
}

void test_desired_heading_zero_distance_satellite_skipped() {
    ShipState s;
    s.pos     = {120.0f, 120.0f};
    s.heading = 0.0f;
    s.exitPt  = {220.0f, 120.0f};
    std::vector<Point2f> sats = {{120.0f, 120.0f}};
    float h = computeDesiredHeading(s, sats);
    TEST_ASSERT_FALSE(std::isnan(h));
    TEST_ASSERT_FALSE(std::isinf(h));
}

void test_desired_heading_satellite_outside_radius_ignored() {
    ShipState s;
    s.pos     = {120.0f, 120.0f};
    s.heading = 0.0f;
    s.exitPt  = {220.0f, 120.0f};
    std::vector<Point2f> sats = {{120.0f, 170.0f}};
    float h = computeDesiredHeading(s, sats);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 0.0f, h);
}

void test_desired_heading_nearby_satellite_deflects() {
    ShipState s;
    s.pos     = {120.0f, 120.0f};
    s.heading = 0.0f;
    s.exitPt  = {220.0f, 120.0f};
    std::vector<Point2f> sats = {{120.0f, 140.0f}};
    float h = computeDesiredHeading(s, sats);
    TEST_ASSERT_TRUE(h < 0.0f);
}

// --- isOutsideBounds ---

void test_inside_bounds_is_false() {
    TEST_ASSERT_FALSE(isOutsideBounds({120.0f, 120.0f}));
}

void test_outside_left_is_true() {
    TEST_ASSERT_TRUE(isOutsideBounds({-6.0f, 120.0f}));
}

void test_outside_right_is_true() {
    TEST_ASSERT_TRUE(isOutsideBounds({246.0f, 120.0f}));
}

void test_outside_top_is_true() {
    TEST_ASSERT_TRUE(isOutsideBounds({120.0f, -6.0f}));
}

void test_outside_bottom_is_true() {
    TEST_ASSERT_TRUE(isOutsideBounds({120.0f, 246.0f}));
}

// --- makeSpawnedState ---

void test_spawn_produces_active_state() {
    ShipState s = makeSpawnedState(0.0f, float(M_PI));
    TEST_ASSERT_TRUE(s.active);
}

void test_spawn_entry_on_disc_rim() {
    ShipState s = makeSpawnedState(0.0f, float(M_PI));
    TEST_ASSERT_FLOAT_WITHIN(0.5f, kCx + kEntryR, s.pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, kCy,            s.pos.y);
}

void test_spawn_exit_outside_screen() {
    ShipState s = makeSpawnedState(0.0f, float(M_PI));
    bool outsideX = s.exitPt.x < -5.0f || s.exitPt.x > 245.0f;
    bool outsideY = s.exitPt.y < -5.0f || s.exitPt.y > 245.0f;
    TEST_ASSERT_TRUE(outsideX || outsideY);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_wrapAngle_positive_overflow);
    RUN_TEST(test_wrapAngle_negative_overflow);
    RUN_TEST(test_wrapAngle_zero_unchanged);
    RUN_TEST(test_stepHeading_within_range_goes_direct);
    RUN_TEST(test_stepHeading_beyond_range_clamps);
    RUN_TEST(test_stepHeading_negative_turn_clamps);
    RUN_TEST(test_desired_heading_no_satellites_points_to_exit);
    RUN_TEST(test_desired_heading_zero_distance_satellite_skipped);
    RUN_TEST(test_desired_heading_satellite_outside_radius_ignored);
    RUN_TEST(test_desired_heading_nearby_satellite_deflects);
    RUN_TEST(test_inside_bounds_is_false);
    RUN_TEST(test_outside_left_is_true);
    RUN_TEST(test_outside_right_is_true);
    RUN_TEST(test_outside_top_is_true);
    RUN_TEST(test_outside_bottom_is_true);
    RUN_TEST(test_spawn_produces_active_state);
    RUN_TEST(test_spawn_entry_on_disc_rim);
    RUN_TEST(test_spawn_exit_outside_screen);
    return UNITY_END();
}
