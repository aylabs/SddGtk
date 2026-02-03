/* blur-processor.c - Gaussian blur processing implementation
 *
 * Copyright (C) 2026 Image Viewer Contributors
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include "blur-processor.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_THREADS 8
#define MIN_THREADS 1
#define MAX_IMAGE_DIMENSION 8192

/* Forward declarations */
static void blur_worker_thread_func(gpointer data, gpointer user_data);
static gboolean blur_completion_idle_callback(gpointer data);
static gboolean ensure_thread_pool_created(BlurProcessor *processor);

/* Private structures */

typedef struct {
    guint request_id;
    GdkPixbuf *source_pixbuf;
    gdouble intensity;
    gboolean is_progressive;
    BlurCompletionCallback callback;
    gpointer user_data;
} BlurWorkItem;

typedef struct {
    BlurCompletionCallback callback;
    gpointer user_data;
    GdkPixbuf *result;
    guint request_id;
    BlurProcessor *processor;
} CallbackData;

struct _BlurProcessor {
    gint max_width;
    gint max_height;
    gint thread_count;
    
    /* Threading infrastructure */
    GThreadPool *thread_pool;
    GAsyncQueue *work_queue;
    GMutex processor_mutex;
    
    /* Request tracking */
    guint next_request_id;
    GHashTable *active_requests;
    
    /* Processing buffers (preallocated for efficiency) */
    guchar *horizontal_buffer;
    guchar *vertical_buffer;
    gsize buffer_size;
    
    gboolean is_destroyed;
};

/* Error quark for blur processing errors */
GQuark blur_error_quark(void) {
    return g_quark_from_static_string("blur-error-quark");
}

/* Utility Functions Implementation */

gdouble blur_calculate_sigma(gdouble intensity) {
    // Clamp intensity to valid range
    if (intensity < 0.0) intensity = 0.0;
    if (intensity > 10.0) intensity = 10.0;
    
    // Formula: sigma = intensity * 2.0
    // Maps intensity [0.0, 10.0] to sigma [0.0, 20.0]
    return intensity * 2.0;
}

gint blur_calculate_kernel_size(gdouble sigma) {
    if (sigma <= 0.0) {
        return 3; // Minimum kernel size
    }
    
    // Formula: size = 2 * ceil(3 * sigma) + 1
    // Covers 99.7% of Gaussian distribution
    gint size = 2 * (gint)ceil(3.0 * sigma) + 1;
    
    // Ensure odd kernel size
    if (size % 2 == 0) {
        size++;
    }
    
    // Clamp to reasonable bounds
    if (size < 3) size = 3;
    if (size > 121) size = 121; // For intensity=10.0, sigma=20.0
    
    return size;
}

gfloat* blur_generate_kernel(gdouble sigma, gint kernel_size) {
    if (sigma <= 0.0 || kernel_size < 3 || kernel_size % 2 == 0) {
        return NULL;
    }
    
    gfloat *kernel = g_malloc(sizeof(gfloat) * kernel_size);
    if (!kernel) {
        return NULL;
    }
    
    gint center = kernel_size / 2;
    gdouble sum = 0.0;
    gdouble two_sigma_squared = 2.0 * sigma * sigma;
    
    // Generate Gaussian values
    for (gint i = 0; i < kernel_size; i++) {
        gint offset = i - center;
        gdouble exponent = -(offset * offset) / two_sigma_squared;
        kernel[i] = (gfloat)exp(exponent);
        sum += kernel[i];
    }
    
    // Normalize kernel so values sum to 1.0
    for (gint i = 0; i < kernel_size; i++) {
        kernel[i] = kernel[i] / sum;
    }
    
    return kernel;
}

gboolean blur_validate_intensity(gdouble intensity) {
    return (intensity >= 0.0 && intensity <= 10.0 && !isnan(intensity) && !isinf(intensity));
}

gboolean blur_validate_pixbuf(GdkPixbuf *pixbuf) {
    if (!pixbuf) {
        return FALSE;
    }
    
    gint width = gdk_pixbuf_get_width(pixbuf);
    gint height = gdk_pixbuf_get_height(pixbuf);
    gint channels = gdk_pixbuf_get_n_channels(pixbuf);
    gint bits_per_sample = gdk_pixbuf_get_bits_per_sample(pixbuf);
    
    // Validate dimensions
    if (width <= 0 || height <= 0 || 
        width > MAX_IMAGE_DIMENSION || height > MAX_IMAGE_DIMENSION) {
        return FALSE;
    }
    
    // Validate format (must be 8-bit RGB or RGBA)
    if (bits_per_sample != 8 || (channels != 3 && channels != 4)) {
        return FALSE;
    }
    
    // Ensure pixbuf has pixel data
    const guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    if (!pixels) {
        return FALSE;
    }
    
    return TRUE;
}

/* Private helper functions */

static gsize calculate_buffer_size(gint width, gint height) {
    // Calculate size for RGBA buffer with alignment padding
    return (gsize)width * height * 4 + 64; // Extra space for alignment
}

static void work_item_free(BlurWorkItem *item) {
    if (item) {
        if (item->source_pixbuf) {
            g_object_unref(item->source_pixbuf);
        }
        g_free(item);
    }
}

/* Core API Implementation */

BlurProcessor* blur_processor_create(gint max_width, gint max_height, gint thread_count) {
    // Validate parameters
    if (max_width <= 0 || max_height <= 0 || 
        max_width > MAX_IMAGE_DIMENSION || max_height > MAX_IMAGE_DIMENSION) {
        return NULL;
    }
    
    if (thread_count < 0) {
        thread_count = g_get_num_processors();
    }
    if (thread_count < MIN_THREADS) thread_count = MIN_THREADS;
    if (thread_count > MAX_THREADS) thread_count = MAX_THREADS;
    
    BlurProcessor *processor = g_malloc0(sizeof(BlurProcessor));
    if (!processor) {
        return NULL;
    }
    
    processor->max_width = max_width;
    processor->max_height = max_height;
    processor->thread_count = thread_count;
    processor->next_request_id = 1;
    processor->is_destroyed = FALSE;
    
    // Initialize synchronization primitives
    g_mutex_init(&processor->processor_mutex);
    
    // Create work queue
    processor->work_queue = g_async_queue_new_full((GDestroyNotify)work_item_free);
    if (!processor->work_queue) {
        g_mutex_clear(&processor->processor_mutex);
        g_free(processor);
        return NULL;
    }
    
    // Create active requests tracking
    processor->active_requests = g_hash_table_new_full(g_direct_hash, g_direct_equal, 
                                                      NULL, NULL);
    if (!processor->active_requests) {
        g_async_queue_unref(processor->work_queue);
        g_mutex_clear(&processor->processor_mutex);
        g_free(processor);
        return NULL;
    }
    
    // Pre-allocate processing buffers for maximum efficiency
    processor->buffer_size = calculate_buffer_size(max_width, max_height);
    processor->horizontal_buffer = g_aligned_alloc(1, processor->buffer_size, 64);
    processor->vertical_buffer = g_aligned_alloc(1, processor->buffer_size, 64);
    
    if (!processor->horizontal_buffer || !processor->vertical_buffer) {
        g_free(processor->horizontal_buffer);
        g_free(processor->vertical_buffer);
        g_hash_table_unref(processor->active_requests);
        g_async_queue_unref(processor->work_queue);
        g_mutex_clear(&processor->processor_mutex);
        g_free(processor);
        return NULL;
    }
    
    return processor;
}

void blur_processor_destroy(BlurProcessor *processor) {
    if (!processor) {
        return;
    }
    
    g_mutex_lock(&processor->processor_mutex);
    processor->is_destroyed = TRUE;
    g_mutex_unlock(&processor->processor_mutex);
    
    // Stop thread pool and wait for completion
    if (processor->thread_pool) {
        g_thread_pool_free(processor->thread_pool, TRUE, TRUE);
        processor->thread_pool = NULL;
    }
    
    // Clean up remaining work items
    BlurWorkItem *item;
    while ((item = g_async_queue_try_pop(processor->work_queue)) != NULL) {
        work_item_free(item);
    }
    
    // Free resources
    g_aligned_free(processor->horizontal_buffer);
    g_aligned_free(processor->vertical_buffer);
    g_hash_table_unref(processor->active_requests);
    g_async_queue_unref(processor->work_queue);
    g_mutex_clear(&processor->processor_mutex);
    
    g_free(processor);
}

/* Placeholder implementations for remaining functions */
/* These will be implemented in subsequent tasks */

/* Separable convolution implementation - T006 & T007 */

static void apply_horizontal_pass(const guchar *src_pixels, guchar *dst_pixels,
                                gint width, gint height, gint rowstride, gint channels,
                                const gfloat *kernel, gint kernel_size) {
    gint half_kernel = kernel_size / 2;
    
    for (gint y = 0; y < height; y++) {
        const guchar *src_row = src_pixels + y * rowstride;
        guchar *dst_row = dst_pixels + y * rowstride;
        
        for (gint x = 0; x < width; x++) {
            gfloat sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f, sum_a = 0.0f;
            
            for (gint k = 0; k < kernel_size; k++) {
                gint sample_x = x + k - half_kernel;
                
                // Mirror edge handling for boundary pixels
                if (sample_x < 0) {
                    sample_x = -sample_x;
                } else if (sample_x >= width) {
                    sample_x = 2 * width - sample_x - 1;
                }
                
                const guchar *sample_pixel = src_row + sample_x * channels;
                gfloat weight = kernel[k];
                
                sum_r += sample_pixel[0] * weight;
                sum_g += sample_pixel[1] * weight;
                sum_b += sample_pixel[2] * weight;
                if (channels == 4) {
                    sum_a += sample_pixel[3] * weight;
                }
            }
            
            // Clamp values to [0, 255] and store
            guchar *dst_pixel = dst_row + x * channels;
            dst_pixel[0] = (guchar)CLAMP(sum_r + 0.5f, 0, 255);
            dst_pixel[1] = (guchar)CLAMP(sum_g + 0.5f, 0, 255);
            dst_pixel[2] = (guchar)CLAMP(sum_b + 0.5f, 0, 255);
            if (channels == 4) {
                dst_pixel[3] = (guchar)CLAMP(sum_a + 0.5f, 0, 255);
            }
        }
    }
}

static void apply_vertical_pass(const guchar *src_pixels, guchar *dst_pixels,
                              gint width, gint height, gint rowstride, gint channels,
                              const gfloat *kernel, gint kernel_size) {
    gint half_kernel = kernel_size / 2;
    
    for (gint y = 0; y < height; y++) {
        guchar *dst_row = dst_pixels + y * rowstride;
        
        for (gint x = 0; x < width; x++) {
            gfloat sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f, sum_a = 0.0f;
            
            for (gint k = 0; k < kernel_size; k++) {
                gint sample_y = y + k - half_kernel;
                
                // Mirror edge handling for boundary pixels
                if (sample_y < 0) {
                    sample_y = -sample_y;
                } else if (sample_y >= height) {
                    sample_y = 2 * height - sample_y - 1;
                }
                
                const guchar *sample_row = src_pixels + sample_y * rowstride;
                const guchar *sample_pixel = sample_row + x * channels;
                gfloat weight = kernel[k];
                
                sum_r += sample_pixel[0] * weight;
                sum_g += sample_pixel[1] * weight;
                sum_b += sample_pixel[2] * weight;
                if (channels == 4) {
                    sum_a += sample_pixel[3] * weight;
                }
            }
            
            // Clamp values to [0, 255] and store
            guchar *dst_pixel = dst_row + x * channels;
            dst_pixel[0] = (guchar)CLAMP(sum_r + 0.5f, 0, 255);
            dst_pixel[1] = (guchar)CLAMP(sum_g + 0.5f, 0, 255);
            dst_pixel[2] = (guchar)CLAMP(sum_b + 0.5f, 0, 255);
            if (channels == 4) {
                dst_pixel[3] = (guchar)CLAMP(sum_a + 0.5f, 0, 255);
            }
        }
    }
}

static GdkPixbuf* apply_separable_gaussian_blur(GdkPixbuf *source_pixbuf, 
                                              gdouble sigma, 
                                              gboolean is_progressive,
                                              guchar *temp_buffer1,
                                              guchar *temp_buffer2) {
    if (!source_pixbuf || sigma <= 0.0) {
        return g_object_ref(source_pixbuf);
    }
    
    gint width = gdk_pixbuf_get_width(source_pixbuf);
    gint height = gdk_pixbuf_get_height(source_pixbuf);
    gint channels = gdk_pixbuf_get_n_channels(source_pixbuf);
    gint rowstride = gdk_pixbuf_get_rowstride(source_pixbuf);
    const guchar *src_pixels = gdk_pixbuf_get_pixels(source_pixbuf);
    
    // Adjust sigma for progressive quality
    gdouble effective_sigma = is_progressive ? sigma * 0.5 : sigma;
    
    // Generate Gaussian kernel
    gint kernel_size = blur_calculate_kernel_size(effective_sigma);
    gfloat *kernel = blur_generate_kernel(effective_sigma, kernel_size);
    if (!kernel) {
        return g_object_ref(source_pixbuf);
    }
    
    // Create result pixbuf
    GdkPixbuf *result = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 
                                      channels == 4,
                                      8, width, height);
    if (!result) {
        g_free(kernel);
        return g_object_ref(source_pixbuf);
    }
    
    guchar *result_pixels = gdk_pixbuf_get_pixels(result);
    gint result_rowstride = gdk_pixbuf_get_rowstride(result);
    
    // Copy source to temp buffer1 for processing
    gsize image_size = height * rowstride;
    memcpy(temp_buffer1, src_pixels, image_size);
    
    // Apply horizontal pass: temp_buffer1 -> temp_buffer2
    apply_horizontal_pass(temp_buffer1, temp_buffer2, 
                         width, height, rowstride, channels, 
                         kernel, kernel_size);
    
    // Apply vertical pass: temp_buffer2 -> result
    apply_vertical_pass(temp_buffer2, result_pixels,
                       width, height, result_rowstride, channels,
                       kernel, kernel_size);
    
    g_free(kernel);
    return result;
}

/* Threading infrastructure - T009 */

typedef struct {
    gpointer data;
} ThreadWorkData;

static void blur_worker_thread_func(gpointer data, gpointer user_data) {
    BlurWorkItem *item = (BlurWorkItem*)data;
    BlurProcessor *processor = (BlurProcessor*)user_data;
    
    if (!item || !processor) {
        return;
    }
    
    // Check if processor is being destroyed
    g_mutex_lock(&processor->processor_mutex);
    if (processor->is_destroyed) {
        g_mutex_unlock(&processor->processor_mutex);
        return;
    }
    g_mutex_unlock(&processor->processor_mutex);
    
    // Perform blur processing
    gdouble sigma = blur_calculate_sigma(item->intensity);
    GdkPixbuf *result = apply_separable_gaussian_blur(
        item->source_pixbuf, 
        sigma, 
        item->is_progressive,
        processor->horizontal_buffer,
        processor->vertical_buffer
    );
    
    // Create completion callback data
    CallbackData *callback_data = g_malloc(sizeof(CallbackData));
    callback_data->callback = item->callback;
    callback_data->user_data = item->user_data;
    callback_data->result = result;
    callback_data->request_id = item->request_id;
    callback_data->processor = processor;
    
    // Schedule callback on main thread
    g_idle_add_full(G_PRIORITY_DEFAULT, 
                   (GSourceFunc)blur_completion_idle_callback, 
                   callback_data, 
                   g_free);
}

static gboolean blur_completion_idle_callback(gpointer data) {
    CallbackData *callback_data = (CallbackData*)data;
    
    // Check if request is still active (not cancelled)
    g_mutex_lock(&callback_data->processor->processor_mutex);
    gboolean request_active = g_hash_table_contains(callback_data->processor->active_requests,
                                                   GUINT_TO_POINTER(callback_data->request_id));
    if (request_active) {
        g_hash_table_remove(callback_data->processor->active_requests,
                           GUINT_TO_POINTER(callback_data->request_id));
    }
    g_mutex_unlock(&callback_data->processor->processor_mutex);
    
    // Only invoke callback if request is still active (not cancelled)
    if (request_active && callback_data->callback) {
        callback_data->callback(callback_data->result, NULL, callback_data->user_data);
    }
    
    // Always clean up the result pixbuf - it was created by the worker thread
    if (callback_data->result) {
        g_object_unref(callback_data->result);
    }
    
    return G_SOURCE_REMOVE;
}

static gboolean ensure_thread_pool_created(BlurProcessor *processor) {
    if (processor->thread_pool) {
        return TRUE;
    }
    
    GError *error = NULL;
    processor->thread_pool = g_thread_pool_new(
        blur_worker_thread_func,
        processor,
        processor->thread_count,
        FALSE, // Don't create exclusive pool
        &error
    );
    
    if (error) {
        g_warning("Failed to create thread pool: %s", error->message);
        g_error_free(error);
        return FALSE;
    }
    
    return processor->thread_pool != NULL;
}

guint blur_processor_apply_async(BlurProcessor *processor,
                               GdkPixbuf *pixbuf,
                               gdouble intensity,
                               gboolean is_progressive,
                               BlurCompletionCallback callback,
                               gpointer user_data) {
    // Input validation - T010
    if (!processor || !pixbuf || !callback) {
        return 0;
    }
    
    if (!blur_validate_intensity(intensity)) {
        GError *error = g_error_new(BLUR_ERROR, BLUR_ERROR_INVALID_INTENSITY,
                                   "Invalid blur intensity: %f", intensity);
        callback(NULL, error, user_data);
        g_error_free(error);
        return 0;
    }
    
    if (!blur_validate_pixbuf(pixbuf)) {
        GError *error = g_error_new(BLUR_ERROR, BLUR_ERROR_INVALID_PIXBUF,
                                   "Invalid pixbuf for blur processing");
        callback(NULL, error, user_data);
        g_error_free(error);
        return 0;
    }
    
    g_mutex_lock(&processor->processor_mutex);
    
    // Check if processor is destroyed
    if (processor->is_destroyed) {
        g_mutex_unlock(&processor->processor_mutex);
        GError *error = g_error_new(BLUR_ERROR, BLUR_ERROR_PROCESSING_FAILED,
                                   "Processor has been destroyed");
        callback(NULL, error, user_data);
        g_error_free(error);
        return 0;
    }
    
    // Handle zero intensity case (no blur needed)
    if (intensity <= 0.0) {
        g_mutex_unlock(&processor->processor_mutex);
        GdkPixbuf *result = g_object_ref(pixbuf);
        callback(result, NULL, user_data);
        return processor->next_request_id++; // Return valid ID even for immediate completion
    }
    
    // Ensure thread pool is created
    if (!ensure_thread_pool_created(processor)) {
        g_mutex_unlock(&processor->processor_mutex);
        GError *error = g_error_new(BLUR_ERROR, BLUR_ERROR_THREAD_FAILED,
                                   "Failed to create thread pool");
        callback(NULL, error, user_data);
        g_error_free(error);
        return 0;
    }
    
    // Create work item
    BlurWorkItem *work_item = g_malloc0(sizeof(BlurWorkItem));
    work_item->request_id = processor->next_request_id++;
    work_item->source_pixbuf = g_object_ref(pixbuf);
    work_item->intensity = intensity;
    work_item->is_progressive = is_progressive;
    work_item->callback = callback;
    work_item->user_data = user_data;
    
    // Add to active requests tracking
    g_hash_table_insert(processor->active_requests,
                       GUINT_TO_POINTER(work_item->request_id),
                       work_item);
    
    guint request_id = work_item->request_id;
    
    g_mutex_unlock(&processor->processor_mutex);
    
    // Submit to thread pool
    GError *error = NULL;
    if (!g_thread_pool_push(processor->thread_pool, work_item, &error)) {
        g_mutex_lock(&processor->processor_mutex);
        g_hash_table_remove(processor->active_requests, GUINT_TO_POINTER(request_id));
        g_mutex_unlock(&processor->processor_mutex);
        
        work_item_free(work_item);
        
        GError *blur_error = g_error_new(BLUR_ERROR, BLUR_ERROR_THREAD_FAILED,
                                        "Failed to submit work to thread pool: %s",
                                        error ? error->message : "Unknown error");
        callback(NULL, blur_error, user_data);
        g_error_free(blur_error);
        if (error) g_error_free(error);
        return 0;
    }
    
    return request_id;
}

gboolean blur_processor_cancel(BlurProcessor *processor, guint request_id) {
    if (!processor || request_id == 0) {
        return FALSE;
    }
    
    g_mutex_lock(&processor->processor_mutex);
    
    BlurWorkItem *work_item = g_hash_table_lookup(processor->active_requests,
                                                 GUINT_TO_POINTER(request_id));
    
    if (work_item) {
        // Remove from active requests - the worker thread completion callback will detect this
        // and not invoke the user callback
        g_hash_table_remove(processor->active_requests, GUINT_TO_POINTER(request_id));
        
        g_mutex_unlock(&processor->processor_mutex);
        return TRUE;
    }
    
    g_mutex_unlock(&processor->processor_mutex);
    return FALSE; // Request not found or already completed
}
