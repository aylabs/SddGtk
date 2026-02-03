#include <check.h>
#include <gtk/gtk.h>
#include "../src/hello-app/hello-application.h"
#include "../src/hello-app/hello-window.h"

/* Test fixtures */
static void
setup(void)
{
    gtk_init();
}

static void
teardown(void)
{
    /* Clean up any test resources */
    while (g_main_context_pending(NULL))
        g_main_context_iteration(NULL, FALSE);
}

/* HelloApplication tests */
START_TEST(test_hello_application_creation)
{
    HelloApplication *app;
    
    app = hello_application_new();
    ck_assert_ptr_nonnull(app);
    ck_assert(HELLO_IS_APPLICATION(app));
    ck_assert(GTK_IS_APPLICATION(app));
    
    g_object_unref(app);
}
END_TEST

START_TEST(test_hello_application_properties)
{
    HelloApplication *app;
    char *app_id;
    GApplicationFlags flags;
    
    app = hello_application_new();
    ck_assert_ptr_nonnull(app);
    
    g_object_get(app, 
                 "application-id", &app_id,
                 "flags", &flags,
                 NULL);
    
    ck_assert_str_eq(app_id, "com.example.HelloApp");
    ck_assert_int_eq(flags, G_APPLICATION_DEFAULT_FLAGS);
    
    g_free(app_id);
    g_object_unref(app);
}
END_TEST

START_TEST(test_hello_application_main_window)
{
    HelloApplication *app;
    GtkWindow *window;
    
    app = hello_application_new();
    ck_assert_ptr_nonnull(app);
    
    /* Initially no main window */
    window = hello_application_get_main_window(app);
    ck_assert_ptr_null(window);
    
    g_object_unref(app);
}
END_TEST

/* Test suite creation */
Suite *
hello_application_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("HelloApplication");
    
    /* Core test case */
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_hello_application_creation);
    tcase_add_test(tc_core, test_hello_application_properties);
    tcase_add_test(tc_core, test_hello_application_main_window);
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

    s = hello_application_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}