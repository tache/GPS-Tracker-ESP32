// Claude Generated: version 1 - Unity tests for SNR color classification and RGB565 conversion
#include <unity.h>
#include "view/SnrColor.h"
using namespace snrcolor;

void setUp() {}
void tearDown() {}

void test_unused_is_red()        { TEST_ASSERT_EQUAL(SatColor::Red,    classify(false, 50)); }
void test_used_strong_is_green() { TEST_ASSERT_EQUAL(SatColor::Green,  classify(true, 35)); }
void test_used_mid_is_orange()   { TEST_ASSERT_EQUAL(SatColor::Orange, classify(true, 20)); }
void test_used_weak_is_yellow()  { TEST_ASSERT_EQUAL(SatColor::Yellow, classify(true, 19)); }
void test_boundary_35_is_green() { TEST_ASSERT_EQUAL(SatColor::Green,  classify(true, 35)); }
void test_boundary_34_is_orange(){ TEST_ASSERT_EQUAL(SatColor::Orange, classify(true, 34)); }
void test_boundary_20_is_orange(){ TEST_ASSERT_EQUAL(SatColor::Orange, classify(true, 20)); }
void test_boundary_19_is_yellow(){ TEST_ASSERT_EQUAL(SatColor::Yellow, classify(true, 19)); }
void test_rgb565_red()    { TEST_ASSERT_EQUAL_HEX16(0xF800, toRgb565(SatColor::Red)); }
void test_rgb565_green()  { TEST_ASSERT_EQUAL_HEX16(0x07E0, toRgb565(SatColor::Green)); }
void test_rgb565_orange() { TEST_ASSERT_EQUAL_HEX16(0xFD20, toRgb565(SatColor::Orange)); }
void test_rgb565_yellow() { TEST_ASSERT_EQUAL_HEX16(0xFFE0, toRgb565(SatColor::Yellow)); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_unused_is_red);
    RUN_TEST(test_used_strong_is_green);
    RUN_TEST(test_used_mid_is_orange);
    RUN_TEST(test_used_weak_is_yellow);
    RUN_TEST(test_boundary_35_is_green);
    RUN_TEST(test_boundary_34_is_orange);
    RUN_TEST(test_boundary_20_is_orange);
    RUN_TEST(test_boundary_19_is_yellow);
    RUN_TEST(test_rgb565_red);
    RUN_TEST(test_rgb565_green);
    RUN_TEST(test_rgb565_orange);
    RUN_TEST(test_rgb565_yellow);
    return UNITY_END();
}
