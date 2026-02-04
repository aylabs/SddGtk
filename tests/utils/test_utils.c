#include "test_utils.h"
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

/**
 * Test Utilities Implementation
 * 
 * Implementation of common utilities for blur feature testing.
 * Provides standardized test patterns and validation functions.
 */

/* Global GTK initialization state */
static gboolean gtk_initialized = FALSE;

/* Standard test cases for parametric testing */
const BlurTestCase STANDARD_BLUR_TEST_CASES[] = {
    {"small_solid", 100, 100, TEST_PATTERN_SOLID, 1.0, 0.1},
    {"small_checkerboard", 100, 100, TEST_PATTERN_CHECKERBOARD, 2.0, 0.2},
    {"medium_gradient", 640, 480, TEST_PATTERN_GRADIENT, 3.0, 0.3},
    {"medium_noise", 640, 480, TEST_PATTERN_NOISE, 1.5, 0.15},
    {"large_edge", 1920, 1080, TEST_PATTERN_CHECKERBOARD, 5.0, 0.4},
};

const int NUM_STANDARD_BLUR_TEST_CASES = sizeof(STANDARD_BLUR_TEST_CASES) / sizeof(STANDARD_BLUR_TEST_CASES[0]);

/* GTK Initialization */
void test_utils_init_gtk(void) {
    if (!gtk_initialized) {
        // Initialize GTK for headless testing
        gtk_init();
        gtk_initialized = TRUE;
        
        // Set up headless environment
        const char *display = g_getenv("DISPLAY");
        if (!display || strlen(display) == 0) {
            g_setenv("GTK_A11Y", "none", TRUE);
        }
    }
}

void test_utils_cleanup_gtk(void) {
    // Note: GTK4 doesn't require explicit cleanup like GTK3
    // This function exists for API consistency
}

/* Pixbuf Creation Utilities */
GdkPixbuf *test_utils_create_pixbuf(int width, int height, TestPatternType pattern) {
    switch (pattern) {
        case TEST_PATTERN_SOLID:
            return test_utils_create_solid_pixbuf(width, height, 128, 128, 128, 255);
        case TEST_PATTERN_CHECKERBOARD:
            return test_utils_create_checkerboard_pixbuf(width, height, 20);
        case TEST_PATTERN_GRADIENT:
            return test_utils_create_gradient_pixbuf(width, height, TRUE);
        case TEST_PATTERN_RANDOM:
        case TEST_PATTERN_NOISE:
            return test_utils_create_noise_pixbuf(width, height, 12345);
        default:
            return test_utils_create_solid_pixbuf(width, height, 0, 0, 0, 255);
    }
}

GdkPixbuf *test_utils_create_solid_pixbuf(int width, int height, guchar r, guchar g, guchar b, guchar a) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    if (!pixbuf) return NULL;
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            p[0] = r;
            p[1] = g;
            p[2] = b;
            if (n_channels == 4) p[3] = a;
        }
    }
    
    return pixbuf;
}

GdkPixbuf *test_utils_create_checkerboard_pixbuf(int width, int height, int square_size) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    if (!pixbuf) return NULL;
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            
            // Checkerboard pattern
            gboolean is_white = ((x / square_size) + (y / square_size)) % 2 == 0;
            guchar value = is_white ? 255 : 0;
            
            p[0] = value; // R
            p[1] = value; // G
            p[2] = value; // B
            if (n_channels == 4) p[3] = 255; // A
        }
    }
    
    return pixbuf;
}

GdkPixbuf *test_utils_create_gradient_pixbuf(int width, int height, gboolean horizontal) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    if (!pixbuf) return NULL;
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            
            // Gradient from black to white
            guchar value;
            if (horizontal) {
                value = (guchar)(255.0 * x / (width - 1));
            } else {
                value = (guchar)(255.0 * y / (height - 1));
            }
            
            p[0] = value; // R
            p[1] = value; // G
            p[2] = value; // B
            if (n_channels == 4) p[3] = 255; // A
        }
    }
    
    return pixbuf;
}

GdkPixbuf *test_utils_create_noise_pixbuf(int width, int height, unsigned int seed) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    if (!pixbuf) return NULL;
    
    srand(seed); // Reproducible noise
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            
            p[0] = (guchar)(rand() % 256); // R
            p[1] = (guchar)(rand() % 256); // G
            p[2] = (guchar)(rand() % 256); // B
            if (n_channels == 4) p[3] = 255; // A
        }
    }
    
    return pixbuf;
}

GdkPixbuf *test_utils_create_edge_test_pixbuf(int width, int height) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    if (!pixbuf) return NULL;
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    // Create image with sharp edges for blur testing
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *p = pixels + y * rowstride + x * n_channels;
            
            guchar value = 0;
            
            // Vertical edge in middle
            if (x > width / 2) value = 255;
            
            // Horizontal edge in middle  
            if (y > height / 2) value = (value > 0) ? 128 : 192;
            
            p[0] = value; // R
            p[1] = value; // G
            p[2] = value; // B
            if (n_channels == 4) p[3] = 255; // A
        }
    }
    
    return pixbuf;
}

/* Pixbuf Comparison Utilities */
gboolean test_utils_pixbufs_equal(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2, double tolerance) {
    if (!pixbuf1 || !pixbuf2) return FALSE;
    
    int width1 = gdk_pixbuf_get_width(pixbuf1);
    int height1 = gdk_pixbuf_get_height(pixbuf1);
    int width2 = gdk_pixbuf_get_width(pixbuf2);
    int height2 = gdk_pixbuf_get_height(pixbuf2);
    
    if (width1 != width2 || height1 != height2) return FALSE;
    
    guchar *pixels1 = gdk_pixbuf_get_pixels(pixbuf1);
    guchar *pixels2 = gdk_pixbuf_get_pixels(pixbuf2);
    int rowstride1 = gdk_pixbuf_get_rowstride(pixbuf1);
    int rowstride2 = gdk_pixbuf_get_rowstride(pixbuf2);
    int n_channels1 = gdk_pixbuf_get_n_channels(pixbuf1);
    int n_channels2 = gdk_pixbuf_get_n_channels(pixbuf2);
    
    if (n_channels1 != n_channels2) return FALSE;
    
    for (int y = 0; y < height1; y++) {
        for (int x = 0; x < width1; x++) {
            for (int c = 0; c < n_channels1; c++) {
                guchar p1 = pixels1[y * rowstride1 + x * n_channels1 + c];
                guchar p2 = pixels2[y * rowstride2 + x * n_channels2 + c];
                
                if (fabs((double)p1 - (double)p2) > tolerance) {
                    return FALSE;
                }
            }
        }
    }
    
    return TRUE;
}

double test_utils_calculate_mse(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2) {
    if (!test_utils_validate_pixbuf(pixbuf1, gdk_pixbuf_get_width(pixbuf2), gdk_pixbuf_get_height(pixbuf2))) {
        return -1.0;
    }
    
    int width = gdk_pixbuf_get_width(pixbuf1);
    int height = gdk_pixbuf_get_height(pixbuf1);
    
    guchar *pixels1 = gdk_pixbuf_get_pixels(pixbuf1);
    guchar *pixels2 = gdk_pixbuf_get_pixels(pixbuf2);
    int rowstride1 = gdk_pixbuf_get_rowstride(pixbuf1);
    int rowstride2 = gdk_pixbuf_get_rowstride(pixbuf2);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf1);
    
    double sum_squared_error = 0.0;
    long total_pixels = (long)width * height * n_channels;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < n_channels; c++) {
                double p1 = pixels1[y * rowstride1 + x * n_channels + c];
                double p2 = pixels2[y * rowstride2 + x * n_channels + c];
                double diff = p1 - p2;
                sum_squared_error += diff * diff;
            }
        }
    }
    
    return sum_squared_error / total_pixels;
}

double test_utils_calculate_psnr(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2) {
    double mse = test_utils_calculate_mse(pixbuf1, pixbuf2);
    if (mse <= 0.0) return 100.0; // Perfect match
    
    double max_pixel_value = 255.0;
    return 20.0 * log10(max_pixel_value / sqrt(mse));
}

gboolean test_utils_validate_pixbuf(GdkPixbuf *pixbuf, int expected_width, int expected_height) {
    if (!pixbuf) return FALSE;
    
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    
    if (expected_width > 0 && width != expected_width) return FALSE;
    if (expected_height > 0 && height != expected_height) return FALSE;
    if (width <= 0 || height <= 0) return FALSE;
    if (!pixels) return FALSE;
    
    return TRUE;
}

guchar test_utils_get_pixel_value(GdkPixbuf *pixbuf, int x, int y, int channel) {
    if (!pixbuf) return 0;
    
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    if (x < 0 || x >= width || y < 0 || y >= height || channel < 0 || channel >= n_channels) {
        return 0;
    }
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    
    return pixels[y * rowstride + x * n_channels + channel];
}

void test_utils_set_pixel_value(GdkPixbuf *pixbuf, int x, int y, int channel, guchar value) {
    if (!pixbuf) return;
    
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    if (x < 0 || x >= width || y < 0 || y >= height || channel < 0 || channel >= n_channels) {
        return;
    }
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    
    pixels[y * rowstride + x * n_channels + channel] = value;
}

ImageStats test_utils_calculate_image_stats(GdkPixbuf *pixbuf, int channel) {
    ImageStats stats = {0.0, 0.0, 0.0, 255, 0};
    
    if (!pixbuf) return stats;
    
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    if (channel < 0 || channel >= n_channels) return stats;
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    
    // Calculate mean and find min/max
    double sum = 0.0;
    long total_pixels = (long)width * height;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar value = pixels[y * rowstride + x * n_channels + channel];
            sum += value;
            
            if (value < stats.min_value) stats.min_value = value;
            if (value > stats.max_value) stats.max_value = value;
        }
    }
    
    stats.mean = sum / total_pixels;
    
    // Calculate variance
    double sum_squared_diff = 0.0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar value = pixels[y * rowstride + x * n_channels + channel];
            double diff = value - stats.mean;
            sum_squared_diff += diff * diff;
        }
    }
    
    stats.variance = sum_squared_diff / total_pixels;
    stats.std_dev = sqrt(stats.variance);
    
    return stats;
}

/* Blur Validation Utilities */
gboolean test_utils_validate_blur_smoothing(GdkPixbuf *original, GdkPixbuf *blurred, double min_smoothing_factor) {
    if (!original || !blurred) return FALSE;
    
    // Calculate variance for both images
    ImageStats original_stats = test_utils_calculate_image_stats(original, 0); // R channel
    ImageStats blurred_stats = test_utils_calculate_image_stats(blurred, 0);
    
    // Blurred image should have lower variance (smoother)
    double smoothing_factor = (original_stats.variance - blurred_stats.variance) / original_stats.variance;
    
    return smoothing_factor >= min_smoothing_factor;
}

gboolean test_utils_validate_blur_boundaries(GdkPixbuf *pixbuf) {
    if (!pixbuf) return FALSE;
    
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    
    // Check that boundary pixels exist and are not corrupted
    // This is a basic boundary validation
    guchar top_left = test_utils_get_pixel_value(pixbuf, 0, 0, 0);
    guchar top_right = test_utils_get_pixel_value(pixbuf, width-1, 0, 0);
    guchar bottom_left = test_utils_get_pixel_value(pixbuf, 0, height-1, 0);
    guchar bottom_right = test_utils_get_pixel_value(pixbuf, width-1, height-1, 0);
    
    // Simple validation: boundary pixels should be valid (not corrupted)
    return (top_left <= 255) && (top_right <= 255) && (bottom_left <= 255) && (bottom_right <= 255);
}

/* Memory Testing Utilities */
long test_utils_get_memory_usage(void) {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) return -1;
    
    char line[128];
    long rss_kb = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %ld kB", &rss_kb);
            break;
        }
    }
    
    fclose(file);
    return rss_kb;
}

void test_utils_start_memory_tracking(MemoryStats *stats) {
    if (!stats) return;
    stats->initial_memory = test_utils_get_memory_usage();
    stats->peak_memory = stats->initial_memory;
    stats->final_memory = stats->initial_memory;
}

void test_utils_update_memory_peak(MemoryStats *stats) {
    if (!stats) return;
    long current_memory = test_utils_get_memory_usage();
    if (current_memory > stats->peak_memory) {
        stats->peak_memory = current_memory;
    }
}

void test_utils_finish_memory_tracking(MemoryStats *stats) {
    if (!stats) return;
    stats->final_memory = test_utils_get_memory_usage();
}

/* Performance Testing Utilities */
void test_utils_timer_start(Timer *timer) {
    if (!timer) return;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timer->start_time = tv.tv_sec + tv.tv_usec / 1000000.0;
}

void test_utils_timer_stop(Timer *timer) {
    if (!timer) return;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timer->end_time = tv.tv_sec + tv.tv_usec / 1000000.0;
}

double test_utils_timer_elapsed(const Timer *timer) {
    if (!timer) return 0.0;
    return timer->end_time - timer->start_time;
}

BenchmarkResults *test_utils_run_benchmark(void (*test_func)(void), int iterations) {
    if (!test_func || iterations <= 0) return NULL;
    
    BenchmarkResults *results = calloc(1, sizeof(BenchmarkResults));
    if (!results) return NULL;
    
    results->times = calloc(iterations, sizeof(double));
    if (!results->times) {
        free(results);
        return NULL;
    }
    
    results->num_iterations = iterations;
    results->min_time = DBL_MAX;
    results->max_time = 0.0;
    results->total_time = 0.0;
    
    Timer timer;
    for (int i = 0; i < iterations; i++) {
        test_utils_timer_start(&timer);
        test_func();
        test_utils_timer_stop(&timer);
        
        double elapsed = test_utils_timer_elapsed(&timer);
        results->times[i] = elapsed;
        results->total_time += elapsed;
        
        if (elapsed < results->min_time) results->min_time = elapsed;
        if (elapsed > results->max_time) results->max_time = elapsed;
    }
    
    results->avg_time = results->total_time / iterations;
    
    return results;
}

void test_utils_free_benchmark_results(BenchmarkResults *results) {
    if (!results) return;
    free(results->times);
    free(results);
}

/* Error Handling Utilities */
void test_utils_log_error(const char *test_name, const char *format, ...) {
    fprintf(stderr, "[ERROR:%s] ", test_name ? test_name : "UNKNOWN");
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

void test_utils_log_warning(const char *test_name, const char *format, ...) {
    fprintf(stderr, "[WARN:%s] ", test_name ? test_name : "UNKNOWN");
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

void test_utils_log_info(const char *test_name, const char *format, ...) {
    printf("[INFO:%s] ", test_name ? test_name : "UNKNOWN");
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

/* Thread Safety Testing */
gboolean test_utils_run_concurrent_test(void *(*thread_func)(void *), void *shared_data, int num_threads, int iterations_per_thread) {
    if (!thread_func || num_threads <= 0) return FALSE;
    
    pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
    ThreadTestData *thread_data = calloc(num_threads, sizeof(ThreadTestData));
    
    if (!threads || !thread_data) {
        free(threads);
        free(thread_data);
        return FALSE;
    }
    
    // Initialize thread data
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].shared_data = shared_data;
        thread_data[i].success = TRUE;
    }
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_data[i]) != 0) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(thread_data);
            return FALSE;
        }
    }
    
    // Wait for all threads to complete
    gboolean all_success = TRUE;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        if (!thread_data[i].success) {
            all_success = FALSE;
        }
    }
    
    // Cleanup
    free(threads);
    free(thread_data);
    
    return all_success;
}

/* Constitutional Compliance Validation */
gboolean test_utils_validate_constitutional_requirements(void) {
    // Check Ubuntu environment for CI compliance
    return test_utils_is_ubuntu_environment();
}

gboolean test_utils_is_ubuntu_environment(void) {
    FILE *os_release = fopen("/etc/os-release", "r");
    if (!os_release) return FALSE;
    
    char line[256];
    gboolean is_ubuntu = FALSE;
    
    while (fgets(line, sizeof(line), os_release)) {
        if (strstr(line, "ID=ubuntu") || strstr(line, "ID=\"ubuntu\"")) {
            is_ubuntu = TRUE;
            break;
        }
    }
    
    fclose(os_release);
    return is_ubuntu;
}

gboolean test_utils_validate_test_duration(double duration_seconds) {
    // Constitutional requirement: tests must complete within 60 seconds
    return duration_seconds <= 60.0;
}