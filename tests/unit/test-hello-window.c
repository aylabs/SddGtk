#include <check.h>
#include <gtk/gtk.h>
#include "src/hello-app/hello-application.h"
#include "src/hello-app/hello-window.h"

/* Test fixtures */
static HelloApplication *test_app = NULL;
static GMainLoop *test_loop = NULL;

static gboolean
startup_timeout(gpointer user_data)
{
    g_main_loop_quit((GMainLoop *)user_data);
    return G_SOURCE_REMOVE;
}

static void
setup(void)
{
    gtk_init();
    test_app = hello_application_new();
    
    /* Register and hold the application for testing */
    g_application_register(G_APPLICATION(test_app), NULL, NULL);
    g_application_hold(G_APPLICATION(test_app));
    
    /* Ensure application startup is complete */
    test_loop = g_main_loop_new(NULL, FALSE);
    g_timeout_add(100, startup_timeout, test_loop);
    g_main_loop_run(test_loop);
    g_main_loop_unref(test_loop);
    test_loop = NULL;
}

static void
teardown(void)
{
    if (test_app && G_IS_APPLICATION(test_app)) {
        if (g_application_get_is_registered(G_APPLICATION(test_app))) {
            g_application_release(G_APPLICATION(test_app));
        }
        g_clear_object(&test_app);
    }
    
    /* Clean up any test resources */
    while (g_main_context_pending(NULL))
        g_main_context_iteration(NULL, FALSE);
}

/* HelloWindow tests */
START_TEST(test_hello_window_creation)
{
    HelloWindow *window;
    
    window = hello_window_new(GTK_APPLICATION(test_app));
    ck_assert_ptr_nonnull(window);
    ck_assert(HELLO_IS_WINDOW(window));
    ck_assert(GTK_IS_APPLICATION_WINDOW(window));
    ck_assert(GTK_IS_WINDOW(window));
    
    /* Window will be destroyed automatically when app is released */
}
END_TEST

START_TEST(test_hello_window_properties)
{
    HelloWindow *window;
    char *greeting;
    char *title;
    int width, height;
    
    window = hello_window_new(GTK_APPLICATION(test_app));
    ck_assert_ptr_nonnull(window);
    
    /* Test default properties */
    g_object_get(window, "greeting", &greeting, NULL);
    ck_assert_str_eq(greeting, "Hello World!");
    g_free(greeting);
    
    /* Test window properties */
    title = (char *)gtk_window_get_title(GTK_WINDOW(window));
    ck_assert_str_eq(title, "Hello World");
    
    gtk_window_get_default_size(GTK_WINDOW(window), &width, &height);
    ck_assert_int_eq(width, 400);
    ck_assert_int_eq(height, 300);
    
    /* Window cleanup handled by application teardown */
}
END_TEST

START_TEST(test_hello_window_greeting_property)
{
    HelloWindow *window;
    const char *greeting;
    
    window = hello_window_new(GTK_APPLICATION(test_app));
    ck_assert_ptr_nonnull(window);
    
    /* Test getter */
    greeting = hello_window_get_greeting(window);
    ck_assert_str_eq(greeting, "Hello World!");
    
    /* Test setter */
    hello_window_set_greeting(window, "Test Greeting");
    greeting = hello_window_get_greeting(window);
    ck_assert_str_eq(greeting, "Test Greeting");
    
    /* Test NULL handling */
    hello_window_set_greeting(window, NULL);
    greeting = hello_window_get_greeting(window);
    ck_assert_str_eq(greeting, "Hello World!"); /* Should revert to default */
    
    /* Window cleanup handled by application teardown */
}
END_TEST

/* Test close button functionality */
START_TEST(test_hello_window_close_button)
{
    HelloWindow *window = hello_window_new(GTK_APPLICATION(test_app));
    
    /* Window should not be NULL */
    ck_assert_ptr_nonnull(window);
    
    /* Test that window has close button (implementation detail may vary) */
    /* This is a basic existence test - more complex interaction testing would require DoGTail */
    ck_assert(GTK_IS_WINDOW(window));
    
    /* Window cleanup handled by application teardown */
}
END_TEST

/* Test suite creation */
Suite *
hello_window_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("HelloWindow");
    
    /* Core test case */
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_hello_window_creation);
    tcase_add_test(tc_core, test_hello_window_properties);
    tcase_add_test(tc_core, test_hello_window_greeting_property);
    tcase_add_test(tc_core, test_hello_window_close_button);
    suite_add_tcase(s, tc_core);

    return s;
}

/* Main test runner */
int
main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = hello_window_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}