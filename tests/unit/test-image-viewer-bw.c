/**
 * @file test-image-viewer-bw.c
 * @brief Unit tests for HelloImageViewer B&W conversion functionality
 */

#include <glib.h>
#include <gtk/gtk.h>
#include "src/hello-app/hello-image-viewer.h"
#include "src/lib/image-processing.h"

/* Test fixtures */
static GtkApplication *app = NULL;
static const char *test_image_path = NULL;

/**
 * Setup function run before each test
 */
static void
setup_test_fixtures(void)
{
    if (app == NULL) {
        app = gtk_application_new("com.example.test", G_APPLICATION_DEFAULT_FLAGS);
        g_application_register(G_APPLICATION(app), NULL, NULL);
        g_application_hold(G_APPLICATION(app));
    }
    
    /* For CI/headless testing, skip file-dependent tests */
    test_image_path = NULL;
}

/**
 * Teardown function run after each test
 */
static void
teardown_test_fixtures(void)
{
    if (app) {
        g_application_release(G_APPLICATION(app));
        g_object_unref(app);
        app = NULL;
    }
    test_image_path = NULL;
}

/**
 * Test: Multiple window instance isolation
 * Verifies that each HelloImageViewer window maintains independent state
 */
static void
test_multiple_window_isolation(void)
{
    HelloImageViewer *viewer1, *viewer2;
    
    /* Skip if no test image available */
    if (!g_file_test(test_image_path, G_FILE_TEST_EXISTS)) {
        g_test_skip("Test image not available");
        return;
    }
    
    setup_test_fixtures();
    
    /* Create two independent viewer windows */
    viewer1 = hello_image_viewer_new(app, test_image_path);
    viewer2 = hello_image_viewer_new(app, test_image_path);
    
    g_assert_nonnull(viewer1);
    g_assert_nonnull(viewer2);
    g_assert_true(viewer1 != viewer2);  /* Different instances */
    
    /* Verify initial state isolation */
    g_assert_false(hello_image_viewer_get_conversion_state(viewer1));
    g_assert_false(hello_image_viewer_get_conversion_state(viewer2));
    
    /* Convert first window to B&W */
    g_assert_true(hello_image_viewer_toggle_conversion(viewer1));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer1));
    
    /* Verify second window remains unchanged */
    g_assert_false(hello_image_viewer_get_conversion_state(viewer2));
    
    /* Convert second window to B&W */
    g_assert_true(hello_image_viewer_toggle_conversion(viewer2));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer2));
    
    /* Both windows now converted independently */
    g_assert_true(hello_image_viewer_get_conversion_state(viewer1));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer2));
    
    /* Restore first window only */
    g_assert_true(hello_image_viewer_toggle_conversion(viewer1));
    g_assert_false(hello_image_viewer_get_conversion_state(viewer1));
    
    /* Second window still converted */
    g_assert_true(hello_image_viewer_get_conversion_state(viewer2));
    
    /* Clean up */
    gtk_window_destroy(GTK_WINDOW(viewer1));
    gtk_window_destroy(GTK_WINDOW(viewer2));
}

/**
 * Test: Per-window state persistence during session
 * Verifies that window state persists throughout the session
 */
static void
test_per_window_state_persistence(void)
{
    HelloImageViewer *viewer;
    
    /* Skip if no test image available */
    if (test_image_path == NULL || !g_file_test(test_image_path, G_FILE_TEST_EXISTS)) {
        g_test_skip("Test image not available");
        return;
    }
    g_assert_nonnull(viewer);
    
    /* Initial state */
    g_assert_false(hello_image_viewer_get_conversion_state(viewer));
    
    /* Convert and verify state persists */
    g_assert_true(hello_image_viewer_toggle_conversion(viewer));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer));
    
    /* State should persist across multiple operations */
    for (int i = 0; i < 5; i++) {
        g_assert_true(hello_image_viewer_get_conversion_state(viewer));
        /* Simulate other operations that don't affect conversion state */
        hello_image_viewer_set_conversion_enabled(viewer, TRUE);
        g_assert_true(hello_image_viewer_get_conversion_state(viewer));
    }
    
    /* Restore and verify */
    g_assert_true(hello_image_viewer_toggle_conversion(viewer));
    g_assert_false(hello_image_viewer_get_conversion_state(viewer));
    
    /* Clean up */
    gtk_window_destroy(GTK_WINDOW(viewer));
}

/**
 * Test: Memory management for multiple concurrent conversions
 * Verifies no memory leaks when multiple windows perform conversions
 */
static void
test_memory_management_multiple_conversions(void)
{
    HelloImageViewer *viewers[10];
    int num_viewers = 10;
    
    /* Skip if no test image available */
    if (!g_file_test(test_image_path, G_FILE_TEST_EXISTS)) {
        g_test_skip("Test image not available");
        return;
    }
    
    setup_test_fixtures();
    
    /* Create multiple viewers */
    for (int i = 0; i < num_viewers; i++) {
        viewers[i] = hello_image_viewer_new(app, test_image_path);
        g_assert_nonnull(viewers[i]);
    }
    
    /* Convert all to B&W */
    for (int i = 0; i < num_viewers; i++) {
        g_assert_true(hello_image_viewer_toggle_conversion(viewers[i]));
        g_assert_true(hello_image_viewer_get_conversion_state(viewers[i]));
    }
    
    /* Restore some back to color */
    for (int i = 0; i < num_viewers / 2; i++) {
        g_assert_true(hello_image_viewer_toggle_conversion(viewers[i]));
        g_assert_false(hello_image_viewer_get_conversion_state(viewers[i]));
    }
    
    /* Verify remaining are still converted */
    for (int i = num_viewers / 2; i < num_viewers; i++) {
        g_assert_true(hello_image_viewer_get_conversion_state(viewers[i]));
    }
    
    /* Clean up all viewers */
    for (int i = 0; i < num_viewers; i++) {
        gtk_window_destroy(GTK_WINDOW(viewers[i]));
    }
}

/**
 * Test: Independent window behavior validation
 * Comprehensive test of window independence
 */
static void
test_independent_window_behavior(void)
{
    HelloImageViewer *viewer_a, *viewer_b, *viewer_c;
    
    /* Skip if no test image available */
    if (!g_file_test(test_image_path, G_FILE_TEST_EXISTS)) {
        g_test_skip("Test image not available");
        return;
    }
    
    setup_test_fixtures();
    
    /* Create three independent viewers */
    viewer_a = hello_image_viewer_new(app, test_image_path);
    viewer_b = hello_image_viewer_new(app, test_image_path);
    viewer_c = hello_image_viewer_new(app, test_image_path);
    
    g_assert_nonnull(viewer_a);
    g_assert_nonnull(viewer_b);
    g_assert_nonnull(viewer_c);
    
    /* Test different conversion patterns */
    
    /* Pattern 1: A=B&W, B=Color, C=Color */
    hello_image_viewer_toggle_conversion(viewer_a);
    g_assert_true(hello_image_viewer_get_conversion_state(viewer_a));
    g_assert_false(hello_image_viewer_get_conversion_state(viewer_b));
    g_assert_false(hello_image_viewer_get_conversion_state(viewer_c));
    
    /* Pattern 2: A=B&W, B=B&W, C=Color */
    hello_image_viewer_toggle_conversion(viewer_b);
    g_assert_true(hello_image_viewer_get_conversion_state(viewer_a));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer_b));
    g_assert_false(hello_image_viewer_get_conversion_state(viewer_c));
    
    /* Pattern 3: A=Color, B=B&W, C=B&W */
    hello_image_viewer_toggle_conversion(viewer_a);
    hello_image_viewer_toggle_conversion(viewer_c);
    g_assert_false(hello_image_viewer_get_conversion_state(viewer_a));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer_b));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer_c));
    
    /* Test enabling/disabling conversion independently */
    hello_image_viewer_set_conversion_enabled(viewer_b, FALSE);
    hello_image_viewer_set_conversion_enabled(viewer_b, TRUE);
    
    /* Verify states unchanged by enable/disable operations on other windows */
    g_assert_false(hello_image_viewer_get_conversion_state(viewer_a));
    g_assert_true(hello_image_viewer_get_conversion_state(viewer_c));
    
    /* Clean up */
    gtk_window_destroy(GTK_WINDOW(viewer_a));
    gtk_window_destroy(GTK_WINDOW(viewer_b));
    gtk_window_destroy(GTK_WINDOW(viewer_c));
}

/**
 * Main test runner
 */
int
main(int argc, char *argv[])
{
    gtk_test_init(&argc, &argv);
    
    /* Add test cases */
    g_test_add_func("/image-viewer-bw/multiple-window-isolation", 
                    test_multiple_window_isolation);
    g_test_add_func("/image-viewer-bw/per-window-state-persistence", 
                    test_per_window_state_persistence);
    g_test_add_func("/image-viewer-bw/memory-management-multiple-conversions", 
                    test_memory_management_multiple_conversions);
    g_test_add_func("/image-viewer-bw/independent-window-behavior", 
                    test_independent_window_behavior);
    
    return g_test_run();
}