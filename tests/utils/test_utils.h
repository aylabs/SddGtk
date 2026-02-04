#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <gtk/gtk.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Test Utilities for Blur Feature Testing
 * 
 * Common utilities and fixtures for blur testing infrastructure.
 * Provides standardized test patterns and validation functions.
 */

/* Constants for test configuration */
#define TEST_IMAGE_WIDTH_SMALL 100
#define TEST_IMAGE_HEIGHT_SMALL 100
#define TEST_IMAGE_WIDTH_MEDIUM 640
#define TEST_IMAGE_HEIGHT_MEDIUM 480
#define TEST_IMAGE_WIDTH_LARGE 1920
#define TEST_IMAGE_HEIGHT_LARGE 1080

#define TEST_TOLERANCE_DOUBLE 1e-6
#define TEST_TOLERANCE_FLOAT 1e-4f

/* Test pattern types */
typedef enum {
    TEST_PATTERN_SOLID,
    TEST_PATTERN_CHECKERBOARD,
    TEST_PATTERN_GRADIENT,
    TEST_PATTERN_RANDOM,
    TEST_PATTERN_NOISE
} TestPatternType;

/* Test fixture structure */
typedef struct {
    GdkPixbuf *test_pixbuf;
    int width;
    int height;
    TestPatternType pattern;
} TestFixture;

/* Utility function prototypes */

/**
 * Initialize GTK for headless testing
 */
void test_utils_init_gtk(void);

/**
 * Cleanup GTK after testing
 */
void test_utils_cleanup_gtk(void);

/**
 * Create a test pixbuf with specified pattern
 */
GdkPixbuf *test_utils_create_pixbuf(int width, int height, TestPatternType pattern);

/**
 * Create a solid color test pixbuf
 */
GdkPixbuf *test_utils_create_solid_pixbuf(int width, int height, guchar r, guchar g, guchar b, guchar a);

/**
 * Create a checkerboard pattern test pixbuf
 */
GdkPixbuf *test_utils_create_checkerboard_pixbuf(int width, int height, int square_size);

/**
 * Create a gradient test pixbuf
 */
GdkPixbuf *test_utils_create_gradient_pixbuf(int width, int height, gboolean horizontal);

/**
 * Create a noise pattern test pixbuf
 */
GdkPixbuf *test_utils_create_noise_pixbuf(int width, int height, unsigned int seed);

/**
 * Create a pixbuf with edge features for blur testing
 */
GdkPixbuf *test_utils_create_edge_test_pixbuf(int width, int height);

/**
 * Compare two pixbufs for equality within tolerance
 */
gboolean test_utils_pixbufs_equal(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2, double tolerance);

/**
 * Calculate mean squared error between two pixbufs
 */
double test_utils_calculate_mse(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2);

/**
 * Calculate peak signal-to-noise ratio between two pixbufs
 */
double test_utils_calculate_psnr(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2);

/**
 * Validate pixbuf properties
 */
gboolean test_utils_validate_pixbuf(GdkPixbuf *pixbuf, int expected_width, int expected_height);

/**
 * Get pixel value at coordinates (for testing)
 */
guchar test_utils_get_pixel_value(GdkPixbuf *pixbuf, int x, int y, int channel);

/**
 * Set pixel value at coordinates (for testing)
 */
void test_utils_set_pixel_value(GdkPixbuf *pixbuf, int x, int y, int channel, guchar value);

/**
 * Calculate image statistics (mean, variance, etc.)
 */
typedef struct {
    double mean;
    double variance;
    double std_dev;
    guchar min_value;
    guchar max_value;
} ImageStats;

ImageStats test_utils_calculate_image_stats(GdkPixbuf *pixbuf, int channel);

/**
 * Blur validation helper functions
 */

/**
 * Check if blur result has proper smoothing characteristics
 */
gboolean test_utils_validate_blur_smoothing(GdkPixbuf *original, GdkPixbuf *blurred, double min_smoothing_factor);

/**
 * Check if blur preserves image boundaries
 */
gboolean test_utils_validate_blur_boundaries(GdkPixbuf *pixbuf);

/**
 * Memory testing utilities
 */

/**
 * Get current memory usage (RSS in KB)
 */
long test_utils_get_memory_usage(void);

/**
 * Memory leak detection helper
 */
typedef struct {
    long initial_memory;
    long peak_memory;
    long final_memory;
} MemoryStats;

void test_utils_start_memory_tracking(MemoryStats *stats);
void test_utils_update_memory_peak(MemoryStats *stats);
void test_utils_finish_memory_tracking(MemoryStats *stats);

/**
 * Performance testing utilities
 */

/**
 * High-resolution timer
 */
typedef struct {
    double start_time;
    double end_time;
} Timer;

void test_utils_timer_start(Timer *timer);
void test_utils_timer_stop(Timer *timer);
double test_utils_timer_elapsed(const Timer *timer);

/**
 * Benchmark helper for multiple iterations
 */
typedef struct {
    double *times;
    int num_iterations;
    double total_time;
    double avg_time;
    double min_time;
    double max_time;
} BenchmarkResults;

BenchmarkResults *test_utils_run_benchmark(void (*test_func)(void), int iterations);
void test_utils_free_benchmark_results(BenchmarkResults *results);

/**
 * Error handling utilities
 */

/**
 * Custom error logging for tests
 */
void test_utils_log_error(const char *test_name, const char *format, ...);
void test_utils_log_warning(const char *test_name, const char *format, ...);
void test_utils_log_info(const char *test_name, const char *format, ...);

/**
 * Thread safety testing helpers
 */

/**
 * Thread test data structure
 */
typedef struct {
    int thread_id;
    void *shared_data;
    void *result_data;
    gboolean success;
    char *error_message;
} ThreadTestData;

/**
 * Create and run multiple threads for concurrency testing
 */
gboolean test_utils_run_concurrent_test(void *(*thread_func)(void *), void *shared_data, int num_threads, int iterations_per_thread);

/**
 * Validation and assertion helpers
 */

/**
 * Extended assertions for blur testing
 */
#define ck_assert_pixbuf_valid(pixbuf) \
    do { \
        ck_assert_ptr_nonnull(pixbuf); \
        ck_assert_int_gt(gdk_pixbuf_get_width(pixbuf), 0); \
        ck_assert_int_gt(gdk_pixbuf_get_height(pixbuf), 0); \
        ck_assert_ptr_nonnull(gdk_pixbuf_get_pixels(pixbuf)); \
    } while(0)

#define ck_assert_pixbuf_dimensions(pixbuf, expected_width, expected_height) \
    do { \
        ck_assert_pixbuf_valid(pixbuf); \
        ck_assert_int_eq(gdk_pixbuf_get_width(pixbuf), expected_width); \
        ck_assert_int_eq(gdk_pixbuf_get_height(pixbuf), expected_height); \
    } while(0)

#define ck_assert_double_tolerance(actual, expected, tolerance) \
    do { \
        double _diff = fabs((actual) - (expected)); \
        ck_assert_msg(_diff <= (tolerance), \
            "Assertion failed: |%.6f - %.6f| = %.6f > %.6f", \
            (double)(actual), (double)(expected), _diff, (double)(tolerance)); \
    } while(0)

#define ck_assert_memory_no_leak(initial, final, max_growth) \
    do { \
        long _growth = (final) - (initial); \
        ck_assert_msg(_growth <= (max_growth), \
            "Memory leak detected: %ld KB growth > %ld KB maximum", \
            _growth, (long)(max_growth)); \
    } while(0)

#define ck_assert_performance_acceptable(elapsed_time, max_time) \
    do { \
        ck_assert_msg((elapsed_time) <= (max_time), \
            "Performance requirement failed: %.3fs > %.3fs maximum", \
            (double)(elapsed_time), (double)(max_time)); \
    } while(0)

/**
 * Test data generation helpers
 */

/**
 * Generate test cases for parametric testing
 */
typedef struct {
    const char *test_name;
    int width;
    int height;
    TestPatternType pattern;
    double blur_radius;
    double expected_smoothing;
} BlurTestCase;

extern const BlurTestCase STANDARD_BLUR_TEST_CASES[];
extern const int NUM_STANDARD_BLUR_TEST_CASES;

/**
 * Constitutional compliance helpers
 */

/**
 * Validate constitutional requirements for blur testing
 */
gboolean test_utils_validate_constitutional_requirements(void);

/**
 * Check Ubuntu environment for CI compliance
 */
gboolean test_utils_is_ubuntu_environment(void);

/**
 * Validate test execution time requirements
 */
gboolean test_utils_validate_test_duration(double duration_seconds);

#endif /* TEST_UTILS_H */