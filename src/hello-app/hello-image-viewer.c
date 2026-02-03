#include "hello-image-viewer.h"
#include "../lib/image-processing.h"
#include "config.h"
#include <glib/gi18n.h>

struct _HelloImageViewer {
    GtkWindow parent_instance;
    
    /* UI widgets */
    GtkWidget *image_widget;
    GtkWidget *conversion_button;
    
    /* Image data for B&W conversion */
    GdkPixbuf *original_pixbuf;     /* Never modified, session-persistent */
    GdkPixbuf *converted_pixbuf;    /* Created/freed as needed */
    gboolean is_converted;          /* Current conversion state */
    
    /* File information */
    char *current_filename;
};

G_DEFINE_FINAL_TYPE(HelloImageViewer, hello_image_viewer, GTK_TYPE_WINDOW)

/* Default values */
#define DEFAULT_WINDOW_WIDTH  600
#define DEFAULT_WINDOW_HEIGHT 400
#define DEFAULT_WINDOW_TITLE  "Image Viewer"

/* Forward declarations */
static void on_conversion_button_toggled(GtkToggleButton *button, HelloImageViewer *viewer);

static void
hello_image_viewer_dispose(GObject *object)
{
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(object);
    
    /* Clear pixbuf references */
    g_clear_object(&viewer->original_pixbuf);
    g_clear_object(&viewer->converted_pixbuf);
    
    G_OBJECT_CLASS(hello_image_viewer_parent_class)->dispose(object);
}

static void
hello_image_viewer_finalize(GObject *object)
{
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(object);
    
    g_free(viewer->current_filename);
    
    G_OBJECT_CLASS(hello_image_viewer_parent_class)->finalize(object);
}

static void
hello_image_viewer_class_init(HelloImageViewerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    
    object_class->dispose = hello_image_viewer_dispose;
    object_class->finalize = hello_image_viewer_finalize;
    
    /* Bind UI template */
    gtk_widget_class_set_template_from_resource(widget_class,
        "/com/example/HelloApp/resources/hello-image-viewer.ui");
    
    /* Bind template widgets */
    gtk_widget_class_bind_template_child(widget_class, HelloImageViewer, image_widget);
    gtk_widget_class_bind_template_child(widget_class, HelloImageViewer, conversion_button);
    
    /* Bind template signal handlers */
    gtk_widget_class_bind_template_callback(widget_class, on_conversion_button_toggled);
}

static void
hello_image_viewer_init(HelloImageViewer *viewer)
{
    /* Initialize conversion state */
    viewer->original_pixbuf = NULL;
    viewer->converted_pixbuf = NULL;
    viewer->is_converted = FALSE;
    viewer->current_filename = NULL;
    
    /* Initialize template */
    gtk_widget_init_template(GTK_WIDGET(viewer));
    
    /* Set initial button state - disabled until image is loaded */
    gtk_widget_set_sensitive(viewer->conversion_button, FALSE);
}

/**
 * on_conversion_button_toggled:
 * @button: The conversion toggle button
 * @viewer: The HelloImageViewer instance
 * 
 * Handles conversion button toggle events.
 * Converts image between color and grayscale modes.
 */
static void
on_conversion_button_toggled(GtkToggleButton *button, HelloImageViewer *viewer)
{
    GError *error = NULL;
    gboolean is_active = gtk_toggle_button_get_active(button);
    
    g_return_if_fail(HELLO_IS_IMAGE_VIEWER(viewer));
    g_return_if_fail(viewer->original_pixbuf != NULL);
    
    /* Provide processing state feedback - temporarily disable button */
    gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Processing...");
    
    /* Process the UI updates */
    while (g_main_context_pending(NULL)) {
        g_main_context_iteration(NULL, FALSE);
    }
    
    if (is_active) {
        /* Convert to grayscale */
        if (viewer->converted_pixbuf == NULL) {
            viewer->converted_pixbuf = image_processor_convert_to_grayscale(
                viewer->original_pixbuf, &error);
                
            if (viewer->converted_pixbuf == NULL) {
                /* Handle conversion error */
                g_warning("Image conversion failed: %s", 
                         error ? error->message : "Unknown error");
                
                /* Reset button state */
                gtk_toggle_button_set_active(button, FALSE);
                
                /* Re-enable button and restore tooltip */
                gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
                gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Convert to black and white");
                
                /* Show error dialog */
                GtkWidget *dialog = gtk_message_dialog_new(
                    GTK_WINDOW(viewer),
                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "Failed to convert image to black and white");
                    
                if (error) {
                    gtk_message_dialog_format_secondary_text(
                        GTK_MESSAGE_DIALOG(dialog), "%s", error->message);
                    g_error_free(error);
                }
                
                gtk_window_present(GTK_WINDOW(dialog));
                g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
                return;
            }
        }
        
        /* Update display to grayscale */
        gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), 
                              viewer->converted_pixbuf);
        viewer->is_converted = TRUE;
        
        /* Update button appearance */
        gtk_button_set_icon_name(GTK_BUTTON(button), "image-restore-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Restore original colors");
        
        /* Update accessibility state */
        gtk_accessible_update_state(GTK_ACCESSIBLE(button),
                                   GTK_ACCESSIBLE_STATE_PRESSED, TRUE,
                                   -1);
        gtk_accessible_update_property(GTK_ACCESSIBLE(viewer->image_widget),
                                     GTK_ACCESSIBLE_PROPERTY_DESCRIPTION, 
                                     "Image display showing black and white version",
                                     -1);
        
    } else {
        /* Restore original colors */
        gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), 
                              viewer->original_pixbuf);
        viewer->is_converted = FALSE;
        
        /* Update button appearance */
        gtk_button_set_icon_name(GTK_BUTTON(button), "image-filter-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Convert to black and white");
        
        /* Update accessibility state */
        gtk_accessible_update_state(GTK_ACCESSIBLE(button),
                                   GTK_ACCESSIBLE_STATE_PRESSED, FALSE,
                                   -1);
        gtk_accessible_update_property(GTK_ACCESSIBLE(viewer->image_widget),
                                     GTK_ACCESSIBLE_PROPERTY_DESCRIPTION, 
                                     "Image display showing original colors",
                                     -1);
    }
    
    /* Re-enable button after processing is complete */
    gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
}

HelloImageViewer *
hello_image_viewer_new(GtkApplication *app, const char *filename)
{
    HelloImageViewer *viewer;
    
    g_return_val_if_fail(GTK_IS_APPLICATION(app), NULL);
    g_return_val_if_fail(filename != NULL, NULL);
    
    viewer = g_object_new(HELLO_TYPE_IMAGE_VIEWER,
                         "application", app,
                         NULL);
    
    if (!hello_image_viewer_load_image(viewer, filename)) {
        g_object_unref(viewer);
        return NULL;
    }
    
    return viewer;
}

gboolean
hello_image_viewer_load_image(HelloImageViewer *viewer, const char *filename)
{
    GFile *file;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    char *basename;
    char *title;
    
    g_return_val_if_fail(HELLO_IS_IMAGE_VIEWER(viewer), FALSE);
    g_return_val_if_fail(filename != NULL, FALSE);
    
    /* Create GFile from filename */
    file = g_file_new_for_path(filename);
    
    /* Check if file exists and is readable */
    if (!g_file_query_exists(file, NULL)) {
        g_object_unref(file);
        g_warning("Image file does not exist: %s", filename);
        return FALSE;
    }
    
    /* Load pixbuf from file */
    pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    if (pixbuf == NULL) {
        g_object_unref(file);
        g_warning("Failed to load image: %s", error ? error->message : "Unknown error");
        if (error) {
            g_error_free(error);
        }
        return FALSE;
    }
    
    /* Clear previous image data */
    g_clear_object(&viewer->original_pixbuf);
    g_clear_object(&viewer->converted_pixbuf);
    
    /* Store original pixbuf for B&W conversion */
    viewer->original_pixbuf = g_object_ref(pixbuf);
    
    /* Reset conversion state */
    viewer->is_converted = FALSE;
    if (viewer->conversion_button) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(viewer->conversion_button), FALSE);
        gtk_button_set_icon_name(GTK_BUTTON(viewer->conversion_button), "image-filter-symbolic");
        gtk_widget_set_tooltip_text(viewer->conversion_button, "Convert to black and white");
        gtk_widget_set_sensitive(viewer->conversion_button, TRUE);
    }
    
    /* Display the image */
    gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), pixbuf);
    
    /* Update window title with filename */
    basename = g_file_get_basename(file);
    title = g_strdup_printf("%s - %s", basename, DEFAULT_WINDOW_TITLE);
    gtk_window_set_title(GTK_WINDOW(viewer), title);
    
    /* Store current filename */
    g_free(viewer->current_filename);
    viewer->current_filename = g_strdup(filename);
    
    /* Clean up */
    g_free(basename);
    g_free(title);
    g_object_unref(file);
    g_object_unref(pixbuf);
    
    return TRUE;
}

gboolean
hello_image_viewer_toggle_conversion(HelloImageViewer *viewer)
{
    g_return_val_if_fail(HELLO_IS_IMAGE_VIEWER(viewer), FALSE);
    g_return_val_if_fail(viewer->conversion_button != NULL, FALSE);
    g_return_val_if_fail(viewer->original_pixbuf != NULL, FALSE);
    
    /* Toggle the button state, which will trigger the signal handler */
    gboolean current_state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(viewer->conversion_button));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(viewer->conversion_button), !current_state);
    
    return TRUE;
}

gboolean
hello_image_viewer_get_conversion_state(HelloImageViewer *viewer)
{
    g_return_val_if_fail(HELLO_IS_IMAGE_VIEWER(viewer), FALSE);
    
    return viewer->is_converted;
}

void
hello_image_viewer_set_conversion_enabled(HelloImageViewer *viewer, gboolean enabled)
{
    g_return_if_fail(HELLO_IS_IMAGE_VIEWER(viewer));
    g_return_if_fail(viewer->conversion_button != NULL);
    
    if (enabled) {
        /* Enable button if image is loaded */
        gtk_widget_set_sensitive(viewer->conversion_button, 
                                 viewer->original_pixbuf != NULL);
    } else {
        /* Disable button and reset conversion state */
        gtk_widget_set_sensitive(viewer->conversion_button, FALSE);
        
        if (viewer->is_converted) {
            /* Reset to original image */
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(viewer->conversion_button), FALSE);
        }
    }
}
