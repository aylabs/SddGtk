#include <check.h>
#include <gtk/gtk.h>
#include "src/lib/blur-cache.h"

/* Test fixtures */
static BlurCache *test_cache = NULL;

/* Setup function for blur cache tests */
static void setup_blur_cache(void) {
    /* Initialize GTK if not already done */
    if (!gtk_is_initialized()) {
        if (!gtk_init_check()) {
            ck_assert_msg(1, "GTK not available - test skipped");
            return;
        }
    }
    
    /* Create test cache with reasonable limits: 5 entries, 10MB max */
    test_cache = blur_cache_create(5, 10 * 1024 * 1024);
    ck_assert_ptr_nonnull(test_cache);
}

/* Teardown function for blur cache tests */
static void teardown_blur_cache(void) {
    if (test_cache) {
        blur_cache_destroy(test_cache);
        test_cache = NULL;
    }
}

/* Helper function to create test pixbuf */
static GdkPixbuf* create_test_pixbuf(int width, int height, guchar red, guchar green, guchar blue) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    ck_assert_ptr_nonnull(pixbuf);
    
    /* Fill with solid color for predictable testing */
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar *pixel = pixels + y * rowstride + x * n_channels;
            pixel[0] = red;
            pixel[1] = green;
            pixel[2] = blue;
        }
    }
    
    return pixbuf;
}

/* Test: Cache creation and initialization - T021 */
START_TEST(test_cache_creation) {
    BlurCache *cache = blur_cache_create(3, 5 * 1024 * 1024); // 3 entries, 5MB
    ck_assert_ptr_nonnull(cache);
    
    /* Test cache statistics on empty cache */
    BlurCacheStats stats;
    blur_cache_get_stats(cache, &stats);
    ck_assert_int_eq(stats.current_entries, 0);
    ck_assert_int_eq(stats.current_memory, 0);
    ck_assert_int_eq(stats.hit_count, 0);
    ck_assert_int_eq(stats.miss_count, 0);
    
    blur_cache_destroy(cache);
}
END_TEST

/* Test: Cache put operations - T022 */  
START_TEST(test_cache_put_operations) {
    GdkPixbuf *test_pixbuf = create_test_pixbuf(100, 100, 255, 0, 0); // Red pixbuf
    const gchar *test_hash = "test_hash_001";
    gdouble test_intensity = 2.5;
    
    /* Put pixbuf in cache */
    gboolean result = blur_cache_put(test_cache, test_hash, test_intensity, test_pixbuf);
    ck_assert(result == TRUE);
    
    /* Verify cache statistics updated */
    BlurCacheStats stats;
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.current_entries, 1);
    ck_assert(stats.current_memory > 0);
    
    g_object_unref(test_pixbuf);
}
END_TEST

/* Test: Cache get operations - T022 */
START_TEST(test_cache_get_operations) {
    GdkPixbuf *original_pixbuf = create_test_pixbuf(50, 50, 0, 255, 0); // Green pixbuf
    const gchar *test_hash = "test_hash_002";
    gdouble test_intensity = 1.5;
    
    /* Put pixbuf in cache first */
    blur_cache_put(test_cache, test_hash, test_intensity, original_pixbuf);
    
    /* Get pixbuf from cache */
    GdkPixbuf *cached_pixbuf = blur_cache_get(test_cache, test_hash, test_intensity);
    ck_assert_ptr_nonnull(cached_pixbuf);
    
    /* Verify it's the same dimensions */
    ck_assert_int_eq(gdk_pixbuf_get_width(cached_pixbuf), 50);
    ck_assert_int_eq(gdk_pixbuf_get_height(cached_pixbuf), 50);
    
    /* Verify cache hit was recorded */
    BlurCacheStats stats;
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.hit_count, 1);
    
    g_object_unref(original_pixbuf);
    g_object_unref(cached_pixbuf);
}
END_TEST

/* Test: Cache miss behavior - T022 */
START_TEST(test_cache_miss_behavior) {
    const gchar *nonexistent_hash = "nonexistent_hash";
    gdouble test_intensity = 3.0;
    
    /* Try to get non-existent item */
    GdkPixbuf *result = blur_cache_get(test_cache, nonexistent_hash, test_intensity);
    ck_assert_ptr_null(result);
    
    /* Verify cache miss was recorded */
    BlurCacheStats stats;
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.miss_count, 1);
}
END_TEST

/* Test: LRU eviction policy - T023 */
START_TEST(test_lru_eviction_policy) {
    /* Create cache with capacity for only 2 entries */
    BlurCache *small_cache = blur_cache_create(2, 50 * 1024 * 1024); // 2 entries, large memory limit
    
    /* Add first item */
    GdkPixbuf *pixbuf1 = create_test_pixbuf(10, 10, 255, 0, 0);
    blur_cache_put(small_cache, "hash1", 1.0, pixbuf1);
    
    /* Add second item */  
    GdkPixbuf *pixbuf2 = create_test_pixbuf(10, 10, 0, 255, 0);
    blur_cache_put(small_cache, "hash2", 2.0, pixbuf2);
    
    /* Add third item - should evict first item */
    GdkPixbuf *pixbuf3 = create_test_pixbuf(10, 10, 0, 0, 255);
    blur_cache_put(small_cache, "hash3", 3.0, pixbuf3);
    
    /* Verify first item was evicted */
    GdkPixbuf *result1 = blur_cache_get(small_cache, "hash1", 1.0);
    ck_assert_ptr_null(result1);
    
    /* Verify second and third items still exist */
    GdkPixbuf *result2 = blur_cache_get(small_cache, "hash2", 2.0);
    GdkPixbuf *result3 = blur_cache_get(small_cache, "hash3", 3.0);
    ck_assert_ptr_nonnull(result2);
    ck_assert_ptr_nonnull(result3);
    
    /* Cleanup */
    g_object_unref(pixbuf1);
    g_object_unref(pixbuf2);
    g_object_unref(pixbuf3);
    if (result2) g_object_unref(result2);
    if (result3) g_object_unref(result3);
    blur_cache_destroy(small_cache);
}
END_TEST

/* Test: Key generation consistency - T025 */
START_TEST(test_key_generation_consistency) {
    const gchar *hash1 = "consistent_hash";
    const gchar *hash2 = "consistent_hash"; // Same hash
    gdouble intensity1 = 2.5;
    gdouble intensity2 = 2.5; // Same intensity
    
    GdkPixbuf *pixbuf1 = create_test_pixbuf(20, 20, 128, 128, 128);
    GdkPixbuf *pixbuf2 = create_test_pixbuf(20, 20, 64, 64, 64);
    
    /* Put first pixbuf */
    blur_cache_put(test_cache, hash1, intensity1, pixbuf1);
    
    /* Put second pixbuf with same hash+intensity - should replace first */
    blur_cache_put(test_cache, hash2, intensity2, pixbuf2);
    
    /* Should only have one entry */
    BlurCacheStats stats;
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.current_entries, 1);
    
    g_object_unref(pixbuf1);
    g_object_unref(pixbuf2);
}
END_TEST

/* Test: Cache clearing - T027 */
START_TEST(test_cache_clearing) {
    /* Add some items */
    GdkPixbuf *pixbuf1 = create_test_pixbuf(15, 15, 255, 255, 0);
    GdkPixbuf *pixbuf2 = create_test_pixbuf(15, 15, 0, 255, 255);
    
    blur_cache_put(test_cache, "clear_test_1", 1.0, pixbuf1);
    blur_cache_put(test_cache, "clear_test_2", 2.0, pixbuf2);
    
    /* Verify items are there */
    BlurCacheStats stats_before;
    blur_cache_get_stats(test_cache, &stats_before);
    ck_assert_int_eq(stats_before.current_entries, 2);
    
    /* Clear cache */
    blur_cache_clear(test_cache);
    
    /* Verify cache is empty */
    BlurCacheStats stats_after;
    blur_cache_get_stats(test_cache, &stats_after);
    ck_assert_int_eq(stats_after.current_entries, 0);
    ck_assert_int_eq(stats_after.current_memory, 0);
    
    g_object_unref(pixbuf1);
    g_object_unref(pixbuf2);
}
END_TEST

/* Test: Cache statistics - T026 */
START_TEST(test_cache_statistics) {
    GdkPixbuf *pixbuf = create_test_pixbuf(30, 30, 200, 100, 50);
    
    /* Initial stats should be zero */
    BlurCacheStats stats;
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.hit_count, 0);
    ck_assert_int_eq(stats.miss_count, 0);
    
    /* Add item and get it - should record hit */
    blur_cache_put(test_cache, "stats_test", 1.5, pixbuf);
    GdkPixbuf *result = blur_cache_get(test_cache, "stats_test", 1.5);
    
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.hit_count, 1);
    ck_assert_int_eq(stats.miss_count, 0);
    ck_assert_int_eq(stats.current_entries, 1);
    
    /* Try to get non-existent item - should record miss */
    GdkPixbuf *miss_result = blur_cache_get(test_cache, "nonexistent", 2.0);
    
    blur_cache_get_stats(test_cache, &stats);
    ck_assert_int_eq(stats.hit_count, 1);
    ck_assert_int_eq(stats.miss_count, 1);
    
    g_object_unref(pixbuf);
    if (result) g_object_unref(result);
}
END_TEST

/* Test suite creation */
Suite *blur_cache_suite(void) {
    Suite *s;
    TCase *tc_core, *tc_operations, *tc_lru, *tc_stats;
    
    s = suite_create("BlurCache");
    
    /* Core functionality */
    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_cache_creation);
    suite_add_tcase(s, tc_core);
    
    /* Cache operations */
    tc_operations = tcase_create("Operations");  
    tcase_add_test(tc_operations, test_cache_put_operations);
    tcase_add_test(tc_operations, test_cache_get_operations);
    tcase_add_test(tc_operations, test_cache_miss_behavior);
    tcase_add_test(tc_operations, test_cache_clearing);
    tcase_add_checked_fixture(tc_operations, setup_blur_cache, teardown_blur_cache);
    suite_add_tcase(s, tc_operations);
    
    /* LRU behavior */
    tc_lru = tcase_create("LRU");
    tcase_add_test(tc_lru, test_lru_eviction_policy);
    tcase_add_test(tc_lru, test_key_generation_consistency);
    tcase_add_checked_fixture(tc_lru, setup_blur_cache, teardown_blur_cache);
    suite_add_tcase(s, tc_lru);
    
    /* Statistics */
    tc_stats = tcase_create("Statistics");
    tcase_add_test(tc_stats, test_cache_statistics);
    tcase_add_checked_fixture(tc_stats, setup_blur_cache, teardown_blur_cache);
    suite_add_tcase(s, tc_stats);
    
    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;
    
    s = blur_cache_suite();
    sr = srunner_create(s);
    
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}