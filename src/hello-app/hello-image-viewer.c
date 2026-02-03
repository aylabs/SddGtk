#include "hello-image-viewer.h"
#include "../lib/image-processing.h"
#include "../lib/blur-processor.h"
#include "../lib/blur-cache.h"
#include "config.h"
#include <glib/gi18n.h>

struct _HelloImageViewer {
    GtkWindow parent_instance;
    
    /* UI widgets */
    GtkWidget *image_widget;
    GtkWidget *conversion_button;
    
    /* Blur UI widgets - T020 */
    GtkWidget *blur_scale;
    GtkWidget *blur_value_label;
    GtkWidget *blur_icon;
    GtkWidget *blur_container;
    
    /* Image data for B&W conversion */
    GdkPixbuf *original_pixbuf;     /* Never modified, session-persistent */
    GdkPixbuf *converted_pixbuf;    /* Created/freed as needed */
    gboolean is_converted;          /* Current conversion state */
    
    /* Blur processing data - T020 */
    BlurProcessor *blur_processor;
    BlurCache *blur_cache;
    gdouble blur_intensity;         /* Current blur intensity 0.0-10.0 */
    guint blur_timeout_id;          /* Debouncing timer ID */
    guint active_blur_request;      /* Currently processing request ID */
    GdkPixbuf *current_display_pixbuf;  /* Currently displayed image */
    gchar *image_hash;              /* Hash of original image for cache keys */
    
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
static void on_blur_scale_value_changed(GtkScale *scale, HelloImageViewer *viewer);
static gboolean blur_debounce_timeout(gpointer user_data);
static void blur_completion_callback(GdkPixbuf *result_pixbuf, const GError *error, gpointer user_data);
static gchar* calculate_image_hash(GdkPixbuf *pixbuf);
static void update_display_image(HelloImageViewer *viewer);

static void
hello_image_viewer_dispose(GObject *object)
{
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(object);
    
    /* Cancel any active blur processing */
    if (viewer->blur_timeout_id > 0) {
        g_source_remove(viewer->blur_timeout_id);
        viewer->blur_timeout_id = 0;
    }
    
    if (viewer->blur_processor && viewer->active_blur_request > 0) {
        blur_processor_cancel(viewer->blur_processor, viewer->active_blur_request);
        viewer->active_blur_request = 0;
        
        /* Give a moment for the cancel callback to complete */
        while (g_main_context_pending(NULL)) {
            g_main_context_iteration(NULL, FALSE);
        }
    }
    
    /* Clear blur resources with extra safety */
    if (viewer->blur_processor) {
        /* Wait for any pending thread pool operations */
        blur_processor_destroy(viewer->blur_processor);
        viewer->blur_processor = NULL;
    }
    
    if (viewer->blur_cache) {
        blur_cache_destroy(viewer->blur_cache);
        viewer->blur_cache = NULL;
    }
    
    /* Clear pixbuf references */
    g_clear_object(&viewer->original_pixbuf);
    g_clear_object(&viewer->converted_pixbuf);
    g_clear_object(&viewer->current_display_pixbuf);
    
    G_OBJECT_CLASS(hello_image_viewer_parent_class)->dispose(object);
}

static void
hello_image_viewer_finalize(GObject *object)
{
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(object);
    
    g_free(viewer->current_filename);
    g_free(viewer->image_hash);
    
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
    gtk_widget_class_bind_template_child(widget_class, HelloImageViewer, blur_scale);
    gtk_widget_class_bind_template_child(widget_class, HelloImageViewer, blur_value_label);
    gtk_widget_class_bind_template_child(widget_class, HelloImageViewer, blur_icon);
    gtk_widget_class_bind_template_child(widget_class, HelloImageViewer, blur_container);
    
    /* Bind template signal handlers */
    gtk_widget_class_bind_template_callback(widget_class, on_conversion_button_toggled);
    gtk_widget_class_bind_template_callback(widget_class, on_blur_scale_value_changed);
}

static void
hello_image_viewer_init(HelloImageViewer *viewer)
{
    /* Initialize conversion state */
    viewer->original_pixbuf = NULL;
    viewer->converted_pixbuf = NULL;
    viewer->is_converted = FALSE;
    viewer->current_filename = NULL;
    
    /* Initialize blur state - T020 */
    viewer->blur_processor = blur_processor_create(3840, 2160, 0); // Max 4K, auto-detect threads
    viewer->blur_cache = blur_cache_create(5, 150 * 1024 * 1024); // 5 entries, 150MB max
    viewer->blur_intensity = 0.0;
    viewer->blur_timeout_id = 0;
    viewer->active_blur_request = 0;
    viewer->current_display_pixbuf = NULL;
    viewer->image_hash = NULL;
    
    /* Initialize template */
    gtk_widget_init_template(GTK_WIDGET(viewer));
    
    /* Set initial button state - disabled until image is loaded */
    if (viewer->conversion_button) {
        gtk_widget_set_sensitive(viewer->conversion_button, FALSE);
    }
    
    /* Set initial blur controls state */
    if (viewer->blur_scale) {
        gtk_widget_set_sensitive(viewer->blur_scale, FALSE);
    }
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
        viewer->is_converted = TRUE;
        
        /* Clear current blur display to trigger re-blur with new base image */
        g_clear_object(&viewer->current_display_pixbuf);
        
        /* Update display (handles blur if active) - T025 */
        if (viewer->blur_intensity > 0.0) {
            /* Trigger blur on the new B&W image */
            on_blur_scale_value_changed(GTK_SCALE(viewer->blur_scale), viewer);
        } else {
            /* No blur - display B&W image directly */
            update_display_image(viewer);
        }
        
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
        viewer->is_converted = FALSE;
        
        /* Clear current blur display to trigger re-blur with new base image */
        g_clear_object(&viewer->current_display_pixbuf);
        
        /* Update display (handles blur if active) - T025 */
        if (viewer->blur_intensity > 0.0) {
            /* Trigger blur on the original color image */
            on_blur_scale_value_changed(GTK_SCALE(viewer->blur_scale), viewer);
        } else {
            /* No blur - display original image directly */
            update_display_image(viewer);
        }
        
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
    g_clear_object(&viewer->current_display_pixbuf);
    
    /* Clear previous image hash and blur cache entries */
    if (viewer->image_hash) {
        blur_cache_remove(viewer->blur_cache, viewer->image_hash);
        g_free(viewer->image_hash);
    }
    
    /* Store original pixbuf for B&W conversion */
    viewer->original_pixbuf = g_object_ref(pixbuf);
    
    /* Generate new image hash for cache keys */
    viewer->image_hash = calculate_image_hash(pixbuf);
    
    /* Reset conversion state */
    viewer->is_converted = FALSE;
    if (viewer->conversion_button) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(viewer->conversion_button), FALSE);
        gtk_button_set_icon_name(GTK_BUTTON(viewer->conversion_button), "image-filter-symbolic");
        gtk_widget_set_tooltip_text(viewer->conversion_button, "Convert to black and white");
        gtk_widget_set_sensitive(viewer->conversion_button, TRUE);
    }
    
    /* Reset blur state - T026 */
    if (viewer->blur_scale) {
        hello_image_viewer_blur_reset(viewer, FALSE); // Don't clear entire cache, just reset slider
        gtk_widget_set_sensitive(viewer->blur_scale, TRUE);
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

/* Blur functionality implementation - T022, T023, T024 */

/**
 * on_blur_scale_value_changed:
 * @scale: The blur intensity scale widget
 * @viewer: The HelloImageViewer instance
 *
 * Handles blur scale value changes with debouncing for smooth interaction
 */
static void
on_blur_scale_value_changed(GtkScale *scale, HelloImageViewer *viewer)
{
    g_return_if_fail(HELLO_IS_IMAGE_VIEWER(viewer));
    g_return_if_fail(viewer->original_pixbuf != NULL);
    
    gdouble new_intensity = gtk_range_get_value(GTK_RANGE(scale));
    
    /* Update value label immediately */
    gchar *value_text = g_strdup_printf("%.1f", new_intensity);
    gtk_label_set_label(GTK_LABEL(viewer->blur_value_label), value_text);
    g_free(value_text);
    
    /* Store new intensity */
    viewer->blur_intensity = new_intensity;
    
    /* Cancel any pending debounce timeout */
    if (viewer->blur_timeout_id > 0) {
        g_source_remove(viewer->blur_timeout_id);
        viewer->blur_timeout_id = 0;
    }
    
    /* For zero intensity, update immediately */
    if (new_intensity <= 0.0) {
        update_display_image(viewer);
        return;
    }
    
    /* Debounce blur processing to prevent excessive computation */
    viewer->blur_timeout_id = g_timeout_add(100, blur_debounce_timeout, viewer);
}

/**
 * blur_debounce_timeout:
 * @user_data: HelloImageViewer instance
 *
 * Debounce timeout callback that triggers actual blur processing
 */
static gboolean
blur_debounce_timeout(gpointer user_data)
{
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(user_data);
    
    /* Safety check: verify object is still valid */
    if (!HELLO_IS_IMAGE_VIEWER(viewer) || !viewer->blur_processor) {
        return G_SOURCE_REMOVE;
    }
    
    viewer->blur_timeout_id = 0;
    
    /* Cancel any active blur request */
    if (viewer->active_blur_request > 0) {
        blur_processor_cancel(viewer->blur_processor, viewer->active_blur_request);
        viewer->active_blur_request = 0;
    }
    
    /* Get base image (original or B&W converted) */
    GdkPixbuf *base_pixbuf = viewer->is_converted ? 
        viewer->converted_pixbuf : viewer->original_pixbuf;
    
    if (!base_pixbuf) {
        return G_SOURCE_REMOVE;
    }
    
    /* Check cache first */
    GdkPixbuf *cached_result = blur_cache_get(viewer->blur_cache, 
                                             viewer->image_hash, 
                                             viewer->blur_intensity);
    
    if (cached_result) {
        /* Use cached result */
        g_clear_object(&viewer->current_display_pixbuf);
        viewer->current_display_pixbuf = cached_result;
        gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), cached_result);
        return G_SOURCE_REMOVE;
    }
    
    /* Start background blur processing */
    viewer->active_blur_request = blur_processor_apply_async(
        viewer->blur_processor,
        base_pixbuf,
        viewer->blur_intensity,
        FALSE, // Full quality
        blur_completion_callback,
        viewer
    );
    
    return G_SOURCE_REMOVE;
}

/**
 * blur_completion_callback:
 * @result_pixbuf: Blurred pixbuf result (or NULL on error)
 * @error: Error information (or NULL on success)
 * @user_data: HelloImageViewer instance
 *
 * Handles completion of background blur processing
 */
static void
blur_completion_callback(GdkPixbuf *result_pixbuf, const GError *error, gpointer user_data)
{
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(user_data);
    
    /* Safety check: verify object is still valid */
    if (!HELLO_IS_IMAGE_VIEWER(viewer) || !viewer->blur_processor) {
        g_warning("Blur completion callback called on invalid viewer");
        return;
    }
    
    viewer->active_blur_request = 0;
    
    if (error) {
        g_warning("Blur processing failed: %s", error->message);
        return;
    }
    
    if (!result_pixbuf) {
        g_warning("Blur processing returned NULL result");
        return;
    }
    
    /* Cache the result - check if cache still exists */
    if (viewer->blur_cache && viewer->image_hash) {
        blur_cache_put(viewer->blur_cache, viewer->image_hash, 
                       viewer->blur_intensity, result_pixbuf);
    }
    
    /* Update display - check if widgets still exist */
    if (viewer->image_widget && GTK_IS_PICTURE(viewer->image_widget)) {
        g_clear_object(&viewer->current_display_pixbuf);
        viewer->current_display_pixbuf = g_object_ref(result_pixbuf);
        gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), result_pixbuf);
    }
}

/**
 * update_display_image:
 * @viewer: HelloImageViewer instance
 *
 * Updates the displayed image based on current blur and B&W settings
 */
static void
update_display_image(HelloImageViewer *viewer)
{
    g_return_if_fail(HELLO_IS_IMAGE_VIEWER(viewer));
    g_return_if_fail(viewer->original_pixbuf != NULL);
    
    GdkPixbuf *display_pixbuf = NULL;
    
    if (viewer->blur_intensity <= 0.0) {
        /* No blur - use original or converted */
        display_pixbuf = viewer->is_converted ? 
            viewer->converted_pixbuf : viewer->original_pixbuf;
    } else {
        /* Use current display pixbuf (which should be blurred) */
        display_pixbuf = viewer->current_display_pixbuf;
        if (!display_pixbuf) {
            /* Fallback to base image if no blur result yet */
            display_pixbuf = viewer->is_converted ? 
                viewer->converted_pixbuf : viewer->original_pixbuf;
        }
    }
    
    if (display_pixbuf) {
        gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), display_pixbuf);
    }
}

/**
 * calculate_image_hash:
 * @pixbuf: GdkPixbuf to hash
 *
 * Creates a simple hash string for cache key generation
 */
static gchar*
calculate_image_hash(GdkPixbuf *pixbuf)
{
    if (!pixbuf) {
        return g_strdup("null");
    }
    
    gint width = gdk_pixbuf_get_width(pixbuf);
    gint height = gdk_pixbuf_get_height(pixbuf);
    gint channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    /* Simple hash based on dimensions and first few pixels */
    const guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    guint hash = width * height * channels;
    
    if (pixels && width > 0 && height > 0) {
        gint sample_count = MIN(16, width * channels);
        for (gint i = 0; i < sample_count; i++) {
            hash = hash * 31 + pixels[i];
        }
    }
    
    return g_strdup_printf("img_%08x", hash);
}

/* Public API implementation */

gdouble
hello_image_viewer_get_blur_intensity(HelloImageViewer *viewer)
{
    g_return_val_if_fail(HELLO_IS_IMAGE_VIEWER(viewer), -1.0);
    return viewer->blur_intensity;
}

gboolean
hello_image_viewer_set_blur_intensity(HelloImageViewer *viewer, 
                                     gdouble intensity, 
                                     gboolean animate)
{
    g_return_val_if_fail(HELLO_IS_IMAGE_VIEWER(viewer), FALSE);
    g_return_val_if_fail(intensity >= 0.0 && intensity <= 10.0, FALSE);
    
    if (animate) {
        /* Animate slider movement - GTK will handle smooth transition */
        gtk_range_set_value(GTK_RANGE(viewer->blur_scale), intensity);
    } else {
        /* Set directly without animation */
        g_signal_handlers_block_by_func(viewer->blur_scale, 
                                       on_blur_scale_value_changed, viewer);
        gtk_range_set_value(GTK_RANGE(viewer->blur_scale), intensity);
        g_signal_handlers_unblock_by_func(viewer->blur_scale, 
                                         on_blur_scale_value_changed, viewer);
        
        /* Update manually since signal was blocked */
        on_blur_scale_value_changed(GTK_SCALE(viewer->blur_scale), viewer);
    }
    
    return TRUE;
}

gboolean
hello_image_viewer_blur_reset(HelloImageViewer *viewer, gboolean clear_cache)
{
    g_return_val_if_fail(HELLO_IS_IMAGE_VIEWER(viewer), FALSE);
    
    /* Cancel any active processing */
    if (viewer->blur_timeout_id > 0) {
        g_source_remove(viewer->blur_timeout_id);
        viewer->blur_timeout_id = 0;
    }
    
    if (viewer->active_blur_request > 0) {
        blur_processor_cancel(viewer->blur_processor, viewer->active_blur_request);
        viewer->active_blur_request = 0;
    }
    
    /* Clear cache if requested */
    if (clear_cache && viewer->blur_cache) {
        blur_cache_clear(viewer->blur_cache);
    }
    
    /* Reset slider to 0.0 */
    hello_image_viewer_set_blur_intensity(viewer, 0.0, FALSE);
    
    return TRUE;
}
