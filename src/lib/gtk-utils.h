#ifndef GTK_UTILS_H
#define GTK_UTILS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * gtk_utils_init:
 * 
 * Initialize GTK utilities. Should be called after gtk_init().
 */
void gtk_utils_init(void);

/**
 * gtk_utils_cleanup:
 * 
 * Clean up GTK utilities resources.
 */
void gtk_utils_cleanup(void);

/**
 * gtk_utils_center_window_on_screen:
 * @window: A GtkWindow to center
 * 
 * Centers the window on the screen.
 */
void gtk_utils_center_window_on_screen(GtkWindow *window);

/**
 * gtk_utils_apply_css_theme:
 * @widget: Widget to apply theme to, or NULL for application-wide
 * @css_data: CSS string to apply
 * 
 * Applies custom CSS styling to a widget or application-wide.
 */
void gtk_utils_apply_css_theme(GtkWidget *widget, const char *css_data);

/**
 * gtk_utils_show_error_dialog:
 * @parent: Parent window for the dialog
 * @title: Dialog title
 * @message: Error message to display
 * 
 * Shows a simple error dialog.
 */
void gtk_utils_show_error_dialog(GtkWindow *parent, const char *title, const char *message);

/**
 * gtk_utils_get_resource_string:
 * @resource_path: Path to the resource
 * 
 * Gets a string from a GResource.
 * 
 * Returns: (transfer full): The resource content as a string, or NULL on error
 */
char *gtk_utils_get_resource_string(const char *resource_path);

G_END_DECLS

#endif /* GTK_UTILS_H */
