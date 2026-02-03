#ifndef HELLO_IMAGE_VIEWER_H
#define HELLO_IMAGE_VIEWER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HELLO_TYPE_IMAGE_VIEWER (hello_image_viewer_get_type())
G_DECLARE_FINAL_TYPE(HelloImageViewer, hello_image_viewer, HELLO, IMAGE_VIEWER, GtkWindow)

/**
 * hello_image_viewer_new:
 * @app: The GtkApplication instance
 * @filename: Path to the image file to display
 * 
 * Creates a new HelloImageViewer window displaying the specified image.
 * 
 * Returns: (transfer full): A new HelloImageViewer instance
 */
HelloImageViewer *hello_image_viewer_new(GtkApplication *app, const char *filename);

/**
 * hello_image_viewer_load_image:
 * @viewer: A HelloImageViewer instance
 * @filename: Path to the image file to display
 * 
 * Loads and displays an image file in the viewer.
 * 
 * Returns: TRUE if image loaded successfully, FALSE otherwise
 */
gboolean hello_image_viewer_load_image(HelloImageViewer *viewer, const char *filename);

/**
 * hello_image_viewer_toggle_conversion:
 * @viewer: A HelloImageViewer instance
 * 
 * Programmatically toggles between color and B&W modes.
 * Equivalent to user clicking the conversion button.
 * 
 * Returns: TRUE if toggle was successful
 */
gboolean hello_image_viewer_toggle_conversion(HelloImageViewer *viewer);

/**
 * hello_image_viewer_get_conversion_state:
 * @viewer: A HelloImageViewer instance
 * 
 * Gets current image conversion state.
 * 
 * Returns: TRUE if image is currently in B&W mode
 */
gboolean hello_image_viewer_get_conversion_state(HelloImageViewer *viewer);

/**
 * hello_image_viewer_set_conversion_enabled:
 * @viewer: A HelloImageViewer instance
 * @enabled: Whether B&W conversion feature is available
 * 
 * Enables or disables the B&W conversion button.
 * When disabled, button is hidden and conversion state is reset.
 */
void hello_image_viewer_set_conversion_enabled(HelloImageViewer *viewer, gboolean enabled);

G_END_DECLS

#endif /* HELLO_IMAGE_VIEWER_H */
