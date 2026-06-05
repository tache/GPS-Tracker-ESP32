// Claude Generated: version 1 - Unity tests for TPV message fix status parsing
// Claude Generated: version 2 - Update for new signature (fixOut + timeOut); add time tests
#include <unity.h>
#include "model/TpvMessage.h"

void setUp() {}
void tearDown() {}

void test_extracts_fix_3d() {
    std::string fix, time;
    TEST_ASSERT_TRUE(parseTpv(R"({"fix":"3D Fix","time":"2026-04-01T18:32:10Z","lat":37.7})", fix, time));
    TEST_ASSERT_EQUAL_STRING("3D Fix", fix.c_str());
}

void test_extracts_fix_2d() {
    std::string fix, time;
    TEST_ASSERT_TRUE(parseTpv(R"({"fix":"2D Fix"})", fix, time));
    TEST_ASSERT_EQUAL_STRING("2D Fix", fix.c_str());
}

void test_missing_fix_defaults_to_no_fix() {
    std::string fix, time;
    TEST_ASSERT_TRUE(parseTpv(R"({"lat":null})", fix, time));
    TEST_ASSERT_EQUAL_STRING("No Fix", fix.c_str());
}

void test_extracts_zulu_time() {
    std::string fix, time;
    TEST_ASSERT_TRUE(parseTpv(R"({"fix":"3D Fix","time":"2026-04-01T18:32:10Z"})", fix, time));
    TEST_ASSERT_EQUAL_STRING("18:32:10 Z", time.c_str());
}

void test_missing_time_defaults_to_dashes() {
    std::string fix, time;
    TEST_ASSERT_TRUE(parseTpv(R"({"fix":"3D Fix"})", fix, time));
    TEST_ASSERT_EQUAL_STRING("--:--:-- Z", time.c_str());
}

void test_rejects_garbage() {
    std::string fix, time;
    TEST_ASSERT_FALSE(parseTpv("not json", fix, time));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_extracts_fix_3d);
    RUN_TEST(test_extracts_fix_2d);
    RUN_TEST(test_missing_fix_defaults_to_no_fix);
    RUN_TEST(test_extracts_zulu_time);
    RUN_TEST(test_missing_time_defaults_to_dashes);
    RUN_TEST(test_rejects_garbage);
    return UNITY_END();
}
