#ifndef HELLO_APPLICATION_H
#define HELLO_APPLICATION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HELLO_TYPE_APPLICATION (hello_application_get_type())
G_DECLARE_FINAL_TYPE(HelloApplication, hello_application, HELLO, APPLICATION, GtkApplication)

/**
 * hello_application_new:
 * 
 * Creates a new HelloApplication instance.
 * 
 * Returns: (transfer full): A new HelloApplication instance
 */
HelloApplication *hello_application_new(void);

/**
 * hello_application_get_main_window:
 * @app: A HelloApplication instance
 * 
 * Gets the main window for the application.
 * 
 * Returns: (transfer none): The main HelloWindow instance, or NULL if not created
 */
GtkWindow *hello_application_get_main_window(HelloApplication *app);

G_END_DECLS

#endif /* HELLO_APPLICATION_H */
