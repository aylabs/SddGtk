/* blur-processor.h - Gaussian blur processing with background threading
 *
 * Copyright (C) 2026 Image Viewer Contributors
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#pragma once

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

typedef struct _BlurProcessor BlurProcessor;

/**
 * BlurCompletionCallback:
 * @result_pixbuf: Blurred pixbuf on success, NULL on error
 * @error: Error details, NULL on success  
 * @user_data: User data from blur_processor_apply_async()
 *
 * Called when blur processing completes or fails
 * 
 * Note: result_pixbuf ownership transferred to caller, must unreference
 */
typedef void (*BlurCompletionCallback)(GdkPixbuf *result_pixbuf,
                                     const GError *error,
                                     gpointer user_data);

/**
 * BlurErrorCode:
 * @BLUR_ERROR_NONE: No error
 * @BLUR_ERROR_INVALID_INTENSITY: intensity not in [0.0, 10.0]
 * @BLUR_ERROR_INVALID_PIXBUF: NULL or corrupted pixbuf
 * @BLUR_ERROR_MEMORY_ALLOCATION: Failed to allocate memory
 * @BLUR_ERROR_PROCESSING_FAILED: Blur algorithm failed
 * @BLUR_ERROR_THREAD_FAILED: Thread creation/management failed
 * @BLUR_ERROR_CANCELLED: Operation cancelled by user
 *
 * Error codes for blur processing operations
 */
typedef enum {
    BLUR_ERROR_NONE = 0,
    BLUR_ERROR_INVALID_INTENSITY = 1,
    BLUR_ERROR_INVALID_PIXBUF = 2,
    BLUR_ERROR_MEMORY_ALLOCATION = 3,
    BLUR_ERROR_PROCESSING_FAILED = 4,
    BLUR_ERROR_THREAD_FAILED = 5,
    BLUR_ERROR_CANCELLED = 6
} BlurErrorCode;

#define BLUR_ERROR (blur_error_quark())
GQuark blur_error_quark(void);

/* Core API Functions */

/**
 * blur_processor_create:
 * @max_width: Maximum expected image width for buffer optimization
 * @max_height: Maximum expected image height for buffer optimization
 * @thread_count: Number of worker threads (0 for auto-detect)
 *
 * Creates a new blur processor instance with pre-allocated buffers
 * and worker thread pool ready for processing.
 *
 * Returns: New BlurProcessor instance, or NULL on failure
 */
BlurProcessor* blur_processor_create(gint max_width, 
                                   gint max_height, 
                                   gint thread_count);

/**
 * blur_processor_apply_async:
 * @processor: BlurProcessor instance
 * @pixbuf: Source pixbuf to blur (must be valid)
 * @intensity: Blur intensity 0.0-10.0
 * @is_progressive: TRUE for lower quality/faster processing
 * @callback: Completion callback function
 * @user_data: User data passed to callback
 *
 * Applies Gaussian blur asynchronously to a pixbuf. Processing request
 * queued for background execution, callback invoked with result or error.
 *
 * Performance guarantees:
 * - Progressive mode: <50ms for HD images
 * - Full quality mode: <500ms for HD images
 * - Memory overhead: <2x pixbuf size during processing
 *
 * Returns: Request ID for cancellation, or 0 on immediate failure
 */
guint blur_processor_apply_async(BlurProcessor *processor,
                               GdkPixbuf *pixbuf,
                               gdouble intensity,
                               gboolean is_progressive,
                               BlurCompletionCallback callback,
                               gpointer user_data);

/**
 * blur_processor_cancel:
 * @processor: BlurProcessor instance
 * @request_id: Request ID from blur_processor_apply_async()
 *
 * Cancels an in-progress blur operation. Processing stopped as soon 
 * as safely possible, callback invoked with BLUR_ERROR_CANCELLED.
 *
 * Returns: TRUE if cancellation successful, FALSE if already completed
 */
gboolean blur_processor_cancel(BlurProcessor *processor, guint request_id);

/**
 * blur_processor_destroy:
 * @processor: BlurProcessor instance to destroy
 *
 * Destroys blur processor and frees all resources. All worker threads
 * stopped and joined, all memory buffers freed.
 *
 * Precondition: No active processing requests (call cancel first)
 */
void blur_processor_destroy(BlurProcessor *processor);

/* Utility Functions */

/**
 * blur_calculate_sigma:
 * @intensity: Blur intensity 0.0-10.0
 *
 * Calculates Gaussian sigma from intensity parameter.
 * Formula: sigma = intensity * 2.0
 *
 * Returns: Sigma value for Gaussian kernel generation [0.0, 20.0]
 */
gdouble blur_calculate_sigma(gdouble intensity);

/**
 * blur_calculate_kernel_size:
 * @sigma: Gaussian sigma value
 *
 * Calculates optimal kernel size for given sigma.
 * Formula: size = 2 * ceil(3 * sigma) + 1
 *
 * Returns: Odd kernel size covering 99.7% of distribution [3, 121]
 */
gint blur_calculate_kernel_size(gdouble sigma);

/**
 * blur_generate_kernel:
 * @sigma: Gaussian sigma parameter
 * @kernel_size: Size of kernel to generate (must be odd)
 *
 * Generates 1D Gaussian kernel for separable convolution.
 * Kernel values normalized to sum to 1.0.
 *
 * Returns: Dynamically allocated kernel array, caller must free()
 */
gfloat* blur_generate_kernel(gdouble sigma, gint kernel_size);

/**
 * blur_validate_intensity:
 * @intensity: Intensity value to validate
 *
 * Validates blur intensity is within acceptable range.
 *
 * Returns: TRUE if 0.0 <= intensity <= 10.0, FALSE otherwise
 */
gboolean blur_validate_intensity(gdouble intensity);

/**
 * blur_validate_pixbuf:
 * @pixbuf: Pixbuf to validate for blur processing
 *
 * Validates pixbuf is suitable for blur operations.
 *
 * Returns: TRUE if pixbuf valid with acceptable dimensions, FALSE otherwise
 */
gboolean blur_validate_pixbuf(GdkPixbuf *pixbuf);

G_END_DECLS
