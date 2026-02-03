#include "gtk-utils.h"

static GtkCssProvider *global_css_provider = NULL;

void
gtk_utils_init(void)
{
    /* Initialize global CSS provider */
    global_css_provider = gtk_css_provider_new();
}

void
gtk_utils_cleanup(void)
{
    g_clear_object(&global_css_provider);
}

void
gtk_utils_center_window_on_screen(GtkWindow *window)
{
    g_return_if_fail(GTK_IS_WINDOW(window));
    
    /* GTK4 handles window centering automatically through the window manager.
     * This function is provided for API compatibility and future extensions. */
    
    /* We could implement custom centering logic here if needed:
     * - Get display size
     * - Calculate center position
     * - Set window position manually
     * But GTK4's automatic centering is usually sufficient. */
}

void
gtk_utils_apply_css_theme(GtkWidget *widget, const char *css_data)
{
    GtkCssProvider *provider;
    GtkStyleContext *context;
    
    g_return_if_fail(css_data != NULL);
    
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, css_data);
    
    /* Note: gtk_css_provider_load_from_string doesn't use GError in GTK4 */
    
    if (widget) {
        /* Apply to specific widget */
        context = gtk_widget_get_style_context(widget);
        gtk_style_context_add_provider(context, 
                                       GTK_STYLE_PROVIDER(provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } else {
        /* Apply application-wide */
        gtk_style_context_add_provider_for_display(
            gdk_display_get_default(),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    
    g_object_unref(provider);
}

void
gtk_utils_show_error_dialog(GtkWindow *parent, const char *title, const char *message)
{
    GtkAlertDialog *dialog;
    
    g_return_if_fail(title != NULL);
    g_return_if_fail(message != NULL);
    
    dialog = gtk_alert_dialog_new("%s", title);
    gtk_alert_dialog_set_detail(dialog, message);
    
    /* Show dialog asynchronously */
    gtk_alert_dialog_show(dialog, parent);
    
    g_object_unref(dialog);
}

char *
gtk_utils_get_resource_string(const char *resource_path)
{
    GBytes *bytes;
    char *content = NULL;
    GError *error = NULL;
    
    g_return_val_if_fail(resource_path != NULL, NULL);
    
    bytes = g_resources_lookup_data(resource_path, G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
    if (bytes) {
        gsize size;
        gconstpointer data = g_bytes_get_data(bytes, &size);
        content = g_strndup((const char *)data, size);
        g_bytes_unref(bytes);
    } else {
        if (error) {
            g_warning("Failed to load resource '%s': %s", resource_path, error->message);
            g_error_free(error);
        }
    }
    
    return content;
}
