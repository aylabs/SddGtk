#include "hello-application.h"
#include "hello-window.h"
#include "config.h"

struct _HelloApplication {
    GtkApplication parent_instance;
    
    GtkWindow *main_window;
};

G_DEFINE_FINAL_TYPE(HelloApplication, hello_application, GTK_TYPE_APPLICATION)

static void
hello_application_activate(GApplication *app)
{
    HelloApplication *hello_app = HELLO_APPLICATION(app);
    GtkWindow *window;

    g_assert(HELLO_IS_APPLICATION(hello_app));

    /* Get or create the main window */
    window = hello_application_get_main_window(hello_app);
    if (window == NULL) {
        window = GTK_WINDOW(hello_window_new(GTK_APPLICATION(hello_app)));
        hello_app->main_window = window;
    }

    /* Present the window */
    gtk_window_present(window);
}

static void
hello_application_startup(GApplication *app)
{
    /* Chain up to the parent class */
    G_APPLICATION_CLASS(hello_application_parent_class)->startup(app);
    
    /* Set application-level properties */
    g_object_set(app,
                 "application-id", APPLICATION_ID,
                 "flags", G_APPLICATION_DEFAULT_FLAGS,
                 NULL);
}

static void
hello_application_dispose(GObject *object)
{
    HelloApplication *app = HELLO_APPLICATION(object);

    g_clear_object(&app->main_window);

    G_OBJECT_CLASS(hello_application_parent_class)->dispose(object);
}

static void
hello_application_class_init(HelloApplicationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

    object_class->dispose = hello_application_dispose;
    
    app_class->startup = hello_application_startup;
    app_class->activate = hello_application_activate;
}

static void
hello_application_init(HelloApplication *app)
{
    app->main_window = NULL;
}

HelloApplication *
hello_application_new(void)
{
    return g_object_new(HELLO_TYPE_APPLICATION,
                        "application-id", APPLICATION_ID,
                        "flags", G_APPLICATION_DEFAULT_FLAGS,
                        NULL);
}

GtkWindow *
hello_application_get_main_window(HelloApplication *app)
{
    g_return_val_if_fail(HELLO_IS_APPLICATION(app), NULL);
    
    return app->main_window;
}
