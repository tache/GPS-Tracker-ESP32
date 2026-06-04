// Claude Generated: version 1 - Unity tests for TPV message fix status parsing
#include <unity.h>
#include "model/TpvMessage.h"

void setUp() {}
void tearDown() {}

void test_extracts_fix_3d() {
    std::string fix;
    TEST_ASSERT_TRUE(parseTpv(R"({"fix":"3D Fix","lat":37.7})", fix));
    TEST_ASSERT_EQUAL_STRING("3D Fix", fix.c_str());
}

void test_extracts_fix_2d() {
    std::string fix;
    parseTpv(R"({"fix":"2D Fix"})", fix);
    TEST_ASSERT_EQUAL_STRING("2D Fix", fix.c_str());
}

void test_missing_fix_defaults_to_no_fix() {
    std::string fix;
    TEST_ASSERT_TRUE(parseTpv(R"({"lat":null})", fix));
    TEST_ASSERT_EQUAL_STRING("No Fix", fix.c_str());
}

void test_rejects_garbage() {
    std::string fix;
    TEST_ASSERT_FALSE(parseTpv("not json", fix));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_extracts_fix_3d);
    RUN_TEST(test_extracts_fix_2d);
    RUN_TEST(test_missing_fix_defaults_to_no_fix);
    RUN_TEST(test_rejects_garbage);
    return UNITY_END();
}
