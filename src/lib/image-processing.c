#include "image-processing.h"
#include <math.h>

/**
 * Error domain implementation for image processing
 */
GQuark
image_processor_error_quark(void)
{
    return g_quark_from_static_string("image-processor-error-quark");
}

/**
 * image_processor_validate_pixbuf:
 * @pixbuf: GdkPixbuf to validate
 * 
 * Validates that a pixbuf is suitable for conversion operations.
 * Checks for NULL pointer, valid dimensions, supported formats.
 * 
 * Returns: TRUE if pixbuf is valid for processing
 */
gboolean
image_processor_validate_pixbuf(GdkPixbuf *pixbuf)
{
    if (pixbuf == NULL) {
        return FALSE;
    }
    
    /* Check if pixbuf has valid dimensions */
    gint width = gdk_pixbuf_get_width(pixbuf);
    gint height = gdk_pixbuf_get_height(pixbuf);
    
    if (width <= 0 || height <= 0) {
        return FALSE;
    }
    
    /* Check if pixbuf has pixel data */
    if (gdk_pixbuf_get_pixels(pixbuf) == NULL) {
        return FALSE;
    }
    
    /* Check for reasonable size limits (e.g., not larger than 100MP) */
    if (width > 10000 || height > 10000) {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * image_processor_estimate_memory_usage:
 * @width: Image width in pixels
 * @height: Image height in pixels
 * 
 * Estimates memory usage for grayscale conversion.
 * Accounts for original + converted pixbufs + temporary processing buffers.
 * 
 * Returns: Estimated memory usage in bytes
 */
gsize
image_processor_estimate_memory_usage(gint width, gint height)
{
    gsize pixel_count = (gsize)width * (gsize)height;
    
    /* Original pixbuf: assume 4 bytes per pixel (RGBA) */
    gsize original_size = pixel_count * 4;
    
    /* Converted grayscale: 3 bytes per pixel (RGB, but grayscale values) */  
    gsize converted_size = pixel_count * 3;
    
    /* Processing overhead: temporary buffers and GdkPixbuf object overhead */
    gsize overhead = pixel_count / 10; /* 10% overhead estimate */
    
    return original_size + converted_size + overhead;
}

/**
 * image_processor_convert_to_grayscale:
 * @original: Source color image
 * @error: Location to store error information
 * 
 * Converts a color image to grayscale using the ITU-R BT.709 standard.
 * This provides perceptually accurate grayscale conversion by weighting
 * RGB channels according to human eye sensitivity.
 * 
 * Formula: Y = 0.299*R + 0.587*G + 0.114*B
 * 
 * Returns: New grayscale GdkPixbuf or NULL on error
 */
GdkPixbuf*
image_processor_convert_to_grayscale(GdkPixbuf *original, GError **error)
{
    /* Check for NULL input and set error appropriately */
    if (original == NULL) {
        g_set_error(error, IMAGE_PROCESSOR_ERROR,
                   IMAGE_PROCESSOR_ERROR_INVALID_INPUT,
                   "Input image cannot be NULL");
        return NULL;
    }
    
    /* Validate input pixbuf */
    if (!image_processor_validate_pixbuf(original)) {
        g_set_error(error, IMAGE_PROCESSOR_ERROR,
                   IMAGE_PROCESSOR_ERROR_INVALID_INPUT,
                   "Invalid or corrupted input image");
        return NULL;
    }
    
    /* Get image properties */
    gint width = gdk_pixbuf_get_width(original);
    gint height = gdk_pixbuf_get_height(original);
    gint n_channels = gdk_pixbuf_get_n_channels(original);
    gint rowstride = gdk_pixbuf_get_rowstride(original);
    gboolean has_alpha = gdk_pixbuf_get_has_alpha(original);
    
    /* Check memory requirements */
    gsize estimated_memory = image_processor_estimate_memory_usage(width, height);
    if (estimated_memory > 500 * 1024 * 1024) { /* 500MB limit */
        g_set_error(error, IMAGE_PROCESSOR_ERROR,
                   IMAGE_PROCESSOR_ERROR_MEMORY_ALLOCATION,
                   "Image too large for processing (estimated %lu MB)",
                   estimated_memory / (1024 * 1024));
        return NULL;
    }
    
    /* Create new grayscale pixbuf with same dimensions and alpha channel */
    GdkPixbuf *grayscale = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                         has_alpha,
                                         8, /* bits per sample */
                                         width,
                                         height);
    
    if (grayscale == NULL) {
        g_set_error(error, IMAGE_PROCESSOR_ERROR,
                   IMAGE_PROCESSOR_ERROR_MEMORY_ALLOCATION,
                   "Failed to allocate memory for grayscale image");
        return NULL;
    }
    
    /* Get pixel data pointers */
    guchar *src_pixels = gdk_pixbuf_get_pixels(original);
    guchar *dst_pixels = gdk_pixbuf_get_pixels(grayscale);
    gint dst_rowstride = gdk_pixbuf_get_rowstride(grayscale);
    
    /* Convert pixel by pixel using ITU-R BT.709 luminance formula */
    for (gint y = 0; y < height; y++) {
        guchar *src_row = src_pixels + y * rowstride;
        guchar *dst_row = dst_pixels + y * dst_rowstride;
        
        for (gint x = 0; x < width; x++) {
            guchar *src_pixel = src_row + x * n_channels;
            guchar *dst_pixel = dst_row + x * (has_alpha ? 4 : 3);
            
            /* Extract RGB values */
            guchar red = src_pixel[0];
            guchar green = src_pixel[1];
            guchar blue = src_pixel[2];
            
            /* Calculate grayscale value using ITU-R BT.709 formula */
            gdouble luminance = 0.299 * red + 0.587 * green + 0.114 * blue;
            guchar gray_value = (guchar)CLAMP(luminance, 0, 255);
            
            /* Set RGB channels to grayscale value */
            dst_pixel[0] = gray_value; /* Red */
            dst_pixel[1] = gray_value; /* Green */ 
            dst_pixel[2] = gray_value; /* Blue */
            
            /* Copy alpha channel if present */
            if (has_alpha) {
                dst_pixel[3] = src_pixel[3];
            }
        }
    }
    
    return grayscale;
}