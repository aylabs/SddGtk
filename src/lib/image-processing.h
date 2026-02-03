#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

/**
 * ImageProcessorError:
 * @IMAGE_PROCESSOR_ERROR_INVALID_INPUT: NULL or invalid pixbuf provided
 * @IMAGE_PROCESSOR_ERROR_MEMORY_ALLOCATION: Insufficient memory for operation
 * @IMAGE_PROCESSOR_ERROR_CONVERSION_FAILED: Image conversion processing failed
 * @IMAGE_PROCESSOR_ERROR_UNSUPPORTED_FORMAT: Image format not supported
 * 
 * Error domain for image processing operations.
 */
typedef enum {
    IMAGE_PROCESSOR_ERROR_INVALID_INPUT,
    IMAGE_PROCESSOR_ERROR_MEMORY_ALLOCATION, 
    IMAGE_PROCESSOR_ERROR_CONVERSION_FAILED,
    IMAGE_PROCESSOR_ERROR_UNSUPPORTED_FORMAT
} ImageProcessorError;

/**
 * IMAGE_PROCESSOR_ERROR:
 * 
 * Error domain for image processing operations.
 * 
 * Returns: the error domain quark for image processing errors
 */
#define IMAGE_PROCESSOR_ERROR (image_processor_error_quark())
GQuark image_processor_error_quark(void);

/**
 * image_processor_convert_to_grayscale:
 * @original: Source color image (must be valid GdkPixbuf)
 * @error: (nullable): Location to store error information
 * 
 * Converts color image to grayscale using ITU-R BT.709 luminance formula.
 * The formula used is: Y = 0.299*R + 0.587*G + 0.114*B
 * Original pixbuf is never modified. Returns new pixbuf or NULL on error.
 * 
 * Returns: (transfer full) (nullable): New grayscale GdkPixbuf or NULL on error
 */
GdkPixbuf* image_processor_convert_to_grayscale(GdkPixbuf *original, GError **error);

/**
 * image_processor_validate_pixbuf:
 * @pixbuf: (nullable): GdkPixbuf to validate
 * 
 * Validates pixbuf for conversion compatibility.
 * Checks format support, dimensions, and memory requirements.
 * 
 * Returns: TRUE if pixbuf can be safely converted
 */
gboolean image_processor_validate_pixbuf(GdkPixbuf *pixbuf);

/**
 * image_processor_estimate_memory_usage:
 * @width: Image width in pixels
 * @height: Image height in pixels
 * 
 * Estimates total memory usage for conversion including temporary buffers.
 * Used for pre-conversion memory validation.
 * 
 * Returns: Estimated memory usage in bytes
 */
gsize image_processor_estimate_memory_usage(gint width, gint height);

G_END_DECLS

#endif /* IMAGE_PROCESSING_H */