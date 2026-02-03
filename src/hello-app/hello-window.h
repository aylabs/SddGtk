#ifndef HELLO_WINDOW_H
#define HELLO_WINDOW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HELLO_TYPE_WINDOW (hello_window_get_type())
G_DECLARE_FINAL_TYPE(HelloWindow, hello_window, HELLO, WINDOW, GtkApplicationWindow)

/**
 * hello_window_new:
 * @app: The HelloApplication instance
 * 
 * Creates a new HelloWindow instance.
 * 
 * Returns: (transfer full): A new HelloWindow instance
 */
HelloWindow *hello_window_new(GtkApplication *app);

/**
 * hello_window_set_greeting:
 * @window: A HelloWindow instance
 * @text: The greeting text to display
 * 
 * Sets the greeting message displayed in the window.
 */
void hello_window_set_greeting(HelloWindow *window, const char *text);

/**
 * hello_window_get_greeting:
 * @window: A HelloWindow instance
 * 
 * Gets the current greeting message.
 * 
 * Returns: (transfer none): The current greeting text
 */
const char *hello_window_get_greeting(HelloWindow *window);

G_END_DECLS

#endif /* HELLO_WINDOW_H */
