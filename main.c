// main.c — Test harness for exocortex TAP runtime
#include "tap.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_EQ_I(a, b) do { \
    tests_run++; \
    if ((a) == (b)) { tests_passed++; } \
    else { printf("  FAIL line %d: %d != %d\n", __LINE__, (int)(a), (int)(b)); } \
} while(0)

#define ASSERT_EQ_F(a, b, eps) do { \
    tests_run++; \
    if (fabsf((a) - (b)) < (eps)) { tests_passed++; } \
    else { printf("  FAIL line %d: %f != %f\n", __LINE__, (float)(a), (float)(b)); } \
} while(0)

#define ASSERT_EQ_S(a, b) do { \
    tests_run++; \
    if (strcmp((a), (b)) == 0) { tests_passed++; } \
    else { printf("  FAIL line %d: \"%s\" != \"%s\"\n", __LINE__, (a), (b)); } \
} while(0)

int main(void) {
    printf("=== Exocortex WASM Runtime Tests ===\n\n");

    // Test 1: Sense returns 0 for unread sensor
    tap_reset();
    printf("  [1] tap_sense uninitialized returns 0\n");
    ASSERT_EQ_I(tap_sense(0), 0);

    // Test 2: Feed and sense returns latest value
    printf("  [2] tap_sense after feed returns value\n");
    tap_reset();
    tap_feed_sensor(0, 42);
    ASSERT_EQ_I(tap_sense(0), 42);

    // Test 3: Latest value overwrites
    printf("  [3] tap_sense returns latest of multiple feeds\n");
    tap_feed_sensor(0, 100);
    tap_feed_sensor(0, 200);
    ASSERT_EQ_I(tap_sense(0), 200);

    // Test 4: Invalid sensor returns -1
    printf("  [4] tap_sense invalid sensor returns -1\n");
    ASSERT_EQ_I(tap_sense(-1), -1);
    ASSERT_EQ_I(tap_sense(99), -1);

    // Test 5: Store and recall
    printf("  [5] tap_recall after store returns value\n");
    tap_reset();
    tap_store("name", "exocortex");
    ASSERT_EQ_S(tap_recall("name"), "exocortex");

    // Test 6: Recall unknown key
    printf("  [6] tap_recall unknown key returns not found\n");
    ASSERT_EQ_S(tap_recall("missing"), "(not found)");

    // Test 7: Overwrite existing key
    printf("  [7] tap_store overwrites existing value\n");
    tap_store("name", "updated");
    ASSERT_EQ_S(tap_recall("name"), "updated");

    // Test 8: Predict with insufficient data returns current
    printf("  [8] tap_predict with 0 samples returns 0\n");
    tap_reset();
    ASSERT_EQ_F(tap_predict(0), 0.0f, 0.01f);

    // Test 9: Predict with one sample returns that value
    printf("  [9] tap_predict with 1 sample returns that value\n");
    tap_reset();
    tap_feed_sensor(1, 50);
    ASSERT_EQ_F(tap_predict(1), 50.0f, 0.01f);

    // Test 10: Predict with linear trend
    printf("  [10] tap_predict extrapolates linear trend\n");
    tap_reset();
    tap_feed_sensor(2, 10);
    tap_feed_sensor(2, 20);
    // Extrapolation: 20 + (20 - 10) = 30
    ASSERT_EQ_F(tap_predict(2), 30.0f, 0.01f);

    // Test 11: Multiple sensors independent
    printf("  [11] Multiple sensors are independent\n");
    tap_reset();
    tap_feed_sensor(0, 100);
    tap_feed_sensor(1, 200);
    ASSERT_EQ_I(tap_sense(0), 100);
    ASSERT_EQ_I(tap_sense(1), 200);

    // Test 12: Reset clears everything
    printf("  [12] tap_reset clears all state\n");
    tap_reset();
    tap_store("temp", "data");
    tap_feed_sensor(5, 999);
    tap_reset();
    ASSERT_EQ_S(tap_recall("temp"), "(not found)");
    ASSERT_EQ_I(tap_sense(5), 0);

    printf("\n  %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed != tests_run) {
        printf("  SOME TESTS FAILED!\n");
        return 1;
    }
    printf("  ALL TESTS PASSED!\n");
    return 0;
}
