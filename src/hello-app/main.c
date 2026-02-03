#include <gtk/gtk.h>
#include "hello-application.h"
#include "../lib/gtk-utils.h"
#include "config.h"

int
main(int argc, char *argv[])
{
    HelloApplication *app;
    int status;

    /* Initialize GTK */
    gtk_init();
    
    /* Initialize GTK utilities */
    gtk_utils_init();

    /* Create application */
    app = hello_application_new();
    
    /* Run the application */
    status = g_application_run(G_APPLICATION(app), argc, argv);

    /* Cleanup */
    g_object_unref(app);
    gtk_utils_cleanup();

    return status;
}