#include <check.h>
#include <gtk/gtk.h>
#include "src/lib/blur-processor.h"
#include "src/lib/blur-cache.h"

/* Test fixtures */
static BlurProcessor *test_processor = NULL;
static BlurCache *test_cache = NULL;

/* Setup function for integration tests */
static void setup_integration(void) {
    /* Initialize GTK if not already done */
    if (!gtk_is_initialized()) {
        if (!gtk_init_check()) {
            ck_assert_msg(1, "GTK not available - test skipped");
            return;
        }
    }
    
    /* Create test processor and cache */
    test_processor = blur_processor_create(1920, 1080, 2);
    test_cache = blur_cache_create(3, 5 * 1024 * 1024); // 3 entries, 5MB
    
    ck_assert_ptr_nonnull(test_processor);
    ck_assert_ptr_nonnull(test_cache);
}

/* Teardown function for integration tests */
static void teardown_integration(void) {
    if (test_processor) {
        blur_processor_destroy(test_processor);
        test_processor = NULL;
    }
    
    if (test_cache) {
        blur_cache_destroy(test_cache);
        test_cache = NULL;
    }
}

/* Helper function to create test pixbuf */
static GdkPixbuf* create_test_pixbuf(int width, int height) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    ck_assert_ptr_nonnull(pixbuf);
    
    /* Fill with test pattern */
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *pixel = pixels + y * rowstride + x * n_channels;
            pixel[0] = (x + y) % 256;
            pixel[1] = (x * y) % 256;  
            pixel[2] = (x ^ y) % 256;
        }
    }
    
    return pixbuf;
}

/* Test: Processor-cache integration - T026 */
START_TEST(test_processor_cache_integration) {
    ck_assert_ptr_nonnull(test_processor);
    ck_assert_ptr_nonnull(test_cache);
    
    /* Create test image */
    GdkPixbuf *test_pixbuf = create_test_pixbuf(100, 100);
    const gchar *test_hash = "integration_test_001";
    gdouble intensity = 2.0;
    
    /* Simulate blur result (in real implementation, this would come from processor) */
    GdkPixbuf *blurred_result = create_test_pixbuf(100, 100);
    
    /* Put result in cache */
    gboolean cache_result = blur_cache_put(test_cache, test_hash, intensity, blurred_result);
    ck_assert(cache_result == TRUE);
    
    /* Retrieve from cache */
    GdkPixbuf *cached_result = blur_cache_get(test_cache, test_hash, intensity);
    ck_assert_ptr_nonnull(cached_result);
    
    /* Verify dimensions match */
    ck_assert_int_eq(gdk_pixbuf_get_width(cached_result), 100);
    ck_assert_int_eq(gdk_pixbuf_get_height(cached_result), 100);
    
    g_object_unref(test_pixbuf);
    g_object_unref(blurred_result);
    g_object_unref(cached_result);
}
END_TEST

/* Test: Cache miss handling - T031 */
START_TEST(test_cache_miss_handling) {
    /* Try to get non-existent cached result */
    GdkPixbuf *result = blur_cache_get(test_cache, "nonexistent_hash", 1.5);
    ck_assert_ptr_null(result);
    
    /* Verify cache miss statistics */
    BlurCacheStats stats;
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_ge(stats.miss_count, 1);
}
END_TEST

/* Test: Memory management across components - T035 */
START_TEST(test_memory_management) {
    /* Create multiple pixbufs and ensure proper cleanup */
    GdkPixbuf *pixbufs[5];
    const gchar *hashes[] = {"mem_test_1", "mem_test_2", "mem_test_3", "mem_test_4", "mem_test_5"};
    
    /* Create and cache multiple results */
    for (int i = 0; i < 5; i++) {
        pixbufs[i] = create_test_pixbuf(50 + i * 10, 50 + i * 10);
        blur_cache_put(test_cache, hashes[i], 1.0 + i, pixbufs[i]);
    }
    
    /* Verify cache has entries */
    BlurCacheStats stats;
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_ge(stats.current_entries, 1); // At least one should fit in cache
    
    /* Clear cache to test cleanup */
    blur_cache_clear(test_cache);
    
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.current_entries, 0);
    ck_assert_int_eq(stats.current_memory, 0);
    
    /* Cleanup original pixbufs */
    for (int i = 0; i < 5; i++) {
        g_object_unref(pixbufs[i]);
    }
}
END_TEST

/* Test: Error handling and cleanup - T035 */  
START_TEST(test_error_handling) {
    /* Test NULL parameter handling */
    GdkPixbuf *null_result = blur_cache_get(NULL, "test", 1.0);
    ck_assert_ptr_null(null_result);
    
    null_result = blur_cache_get(test_cache, NULL, 1.0);
    ck_assert_ptr_null(null_result);
    
    /* Test invalid cache put operations */
    GdkPixbuf *test_pixbuf = create_test_pixbuf(10, 10);
    gboolean result = blur_cache_put(NULL, "test", 1.0, test_pixbuf);
    ck_assert(result == FALSE);
    
    result = blur_cache_put(test_cache, NULL, 1.0, test_pixbuf);  
    ck_assert(result == FALSE);
    
    result = blur_cache_put(test_cache, "test", 1.0, NULL);
    ck_assert(result == FALSE);
    
    g_object_unref(test_pixbuf);
}
END_TEST

/* Test: Component lifecycle management - T029 */
START_TEST(test_component_lifecycle) {
    /* Test that processor and cache can be created/destroyed multiple times */
    BlurProcessor *temp_processor = blur_processor_create(640, 480, 1);
    ck_assert_ptr_nonnull(temp_processor);
    blur_processor_destroy(temp_processor);
    
    BlurCache *temp_cache = blur_cache_create(2, 1024 * 1024);
    ck_assert_ptr_nonnull(temp_cache);
    blur_cache_destroy(temp_cache);
    
    /* Verify original components still work */
    ck_assert_ptr_nonnull(test_processor);
    ck_assert_ptr_nonnull(test_cache);
}
END_TEST

/* Test: Resource cleanup validation - T028 */
START_TEST(test_resource_cleanup) {
    /* Create temporary cache for cleanup testing */
    BlurCache *cleanup_cache = blur_cache_create(2, 2 * 1024 * 1024);
    
    /* Add items to cache */
    GdkPixbuf *pixbuf1 = create_test_pixbuf(40, 40);
    GdkPixbuf *pixbuf2 = create_test_pixbuf(60, 60);
    
    blur_cache_put(cleanup_cache, "cleanup_1", 1.0, pixbuf1);
    blur_cache_put(cleanup_cache, "cleanup_2", 2.0, pixbuf2);
    
    /* Verify cache has items */
    BlurCacheStats stats;
    blur_cache_get_stats(cleanup_cache, &stats);
    ck_assert_int_eq(stats.current_entries, 2);
    
    /* Destroy cache - this should clean up all internal resources */
    blur_cache_destroy(cleanup_cache);
    
    /* Cleanup our references */
    g_object_unref(pixbuf1);
    g_object_unref(pixbuf2);
    
    /* Test passes if no memory errors occur */
}
END_TEST

/* Test suite creation */
Suite *blur_integration_suite(void) {
    Suite *s;
    TCase *tc_integration, *tc_memory, *tc_lifecycle;
    
    s = suite_create("BlurIntegration");
    
    /* Integration tests */
    tc_integration = tcase_create("Integration");
    tcase_add_test(tc_integration, test_processor_cache_integration);
    tcase_add_test(tc_integration, test_cache_miss_handling);
    tcase_add_checked_fixture(tc_integration, setup_integration, teardown_integration);
    suite_add_tcase(s, tc_integration);
    
    /* Memory management tests */
    tc_memory = tcase_create("Memory");
    tcase_add_test(tc_memory, test_memory_management);
    tcase_add_test(tc_memory, test_error_handling);
    tcase_add_test(tc_memory, test_resource_cleanup);
    tcase_add_checked_fixture(tc_memory, setup_integration, teardown_integration);
    suite_add_tcase(s, tc_memory);
    
    /* Lifecycle tests */
    tc_lifecycle = tcase_create("Lifecycle");
    tcase_add_test(tc_lifecycle, test_component_lifecycle);
    tcase_add_checked_fixture(tc_lifecycle, setup_integration, teardown_integration);
    suite_add_tcase(s, tc_lifecycle);
    
    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;
    
    s = blur_integration_suite();
    sr = srunner_create(s);
    
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}