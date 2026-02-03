# Image Blur Library API Contracts

## Core Blur Processing API

### blur_processor_create()
```c
/**
 * Creates a new blur processor instance
 * 
 * @param max_width Maximum expected image width for buffer optimization
 * @param max_height Maximum expected image height for buffer optimization  
 * @param thread_count Number of worker threads (0 for auto-detect)
 * @return New BlurProcessor instance, or NULL on failure
 * 
 * @preconditions:
 * - max_width > 0 && max_width <= 8192
 * - max_height > 0 && max_height <= 8192
 * - thread_count >= 0 && thread_count <= 8
 * 
 * @postconditions:
 * - Returns initialized processor with pre-allocated buffers
 * - Worker thread pool created and ready
 * - Internal caching structures initialized
 * 
 * @errors:
 * - Returns NULL if memory allocation fails
 * - Returns NULL if thread creation fails
 */
BlurProcessor* blur_processor_create(gint max_width, gint max_height, gint thread_count);
```

### blur_processor_apply_async()
```c
/**
 * Applies Gaussian blur asynchronously to a pixbuf
 * 
 * @param processor BlurProcessor instance 
 * @param pixbuf Source pixbuf to blur (must be valid)
 * @param intensity Blur intensity 0.0-10.0
 * @param is_progressive TRUE for lower quality/faster processing
 * @param callback Completion callback function
 * @param user_data User data passed to callback
 * @return Request ID for cancellation, or 0 on immediate failure
 * 
 * @preconditions:
 * - processor != NULL && processor is valid
 * - pixbuf != NULL && pixbuf has valid dimensions
 * - 0.0 <= intensity <= 10.0
 * - callback != NULL
 * 
 * @postconditions:
 * - Processing request queued for background execution
 * - callback will be invoked with result or error
 * - Original pixbuf remains unchanged
 * 
 * @performance:
 * - Progressive mode: <50ms for HD images
 * - Full quality mode: <500ms for HD images
 * - Memory overhead: <2x pixbuf size during processing
 */
guint blur_processor_apply_async(BlurProcessor *processor,
                                GdkPixbuf *pixbuf,
                                gdouble intensity,
                                gboolean is_progressive,
                                BlurCompletionCallback callback,
                                gpointer user_data);
```

### blur_processor_cancel()
```c
/**
 * Cancels an in-progress blur operation
 * 
 * @param processor BlurProcessor instance
 * @param request_id Request ID from blur_processor_apply_async()
 * @return TRUE if cancellation successful, FALSE if already completed
 * 
 * @preconditions:
 * - processor != NULL
 * - request_id > 0 and corresponds to active request
 * 
 * @postconditions:
 * - Processing stopped as soon as safely possible
 * - Callback invoked with BLUR_ERROR_CANCELLED
 * - Partial results discarded
 */
gboolean blur_processor_cancel(BlurProcessor *processor, guint request_id);
```

### blur_processor_destroy()
```c
/**
 * Destroys blur processor and frees all resources
 * 
 * @param processor BlurProcessor instance to destroy
 * 
 * @preconditions:
 * - No active processing requests (call cancel first)
 * 
 * @postconditions:
 * - All worker threads stopped and joined
 * - All memory buffers freed
 * - processor pointer becomes invalid
 */
void blur_processor_destroy(BlurProcessor *processor);
```

## Callback Function Signatures

### BlurCompletionCallback
```c
/**
 * Called when blur processing completes or fails
 * 
 * @param result_pixbuf Blurred pixbuf on success, NULL on error
 * @param error Error details, NULL on success
 * @param user_data User data from blur_processor_apply_async()
 * 
 * @ownership:
 * - result_pixbuf: Caller must unreference with g_object_unref()
 * - error: Owned by blur processor, valid only during callback
 */
typedef void (*BlurCompletionCallback)(GdkPixbuf *result_pixbuf,
                                     const GError *error,
                                     gpointer user_data);
```

## Cache Management API

### blur_cache_create()
```c
/**
 * Creates LRU cache for blur results
 * 
 * @param max_entries Maximum number of cached blur variants
 * @param max_memory_bytes Maximum total memory usage in bytes
 * @return New BlurCache instance, or NULL on failure
 * 
 * @preconditions:
 * - max_entries > 0 && max_entries <= 20
 * - max_memory_bytes >= 10MB && max_memory_bytes <= 1GB
 * 
 * @postconditions:
 * - Empty cache ready for use
 * - Memory tracking initialized to 0
 */
BlurCache* blur_cache_create(guint max_entries, gsize max_memory_bytes);
```

### blur_cache_get()
```c
/**
 * Retrieves cached blur result if available
 * 
 * @param cache BlurCache instance
 * @param pixbuf_hash Hash of original pixbuf
 * @param intensity Blur intensity to look up
 * @return Cached pixbuf with added reference, or NULL if not found
 * 
 * @preconditions:
 * - cache != NULL
 * - pixbuf_hash is valid hash string
 * - intensity matches cache precision (0.1)
 * 
 * @postconditions:
 * - Cache hit updates LRU order
 * - Returned pixbuf has incremented reference count
 * 
 * @performance:
 * - Cache lookup: O(1) average case
 * - LRU update: O(1) amortized
 */
GdkPixbuf* blur_cache_get(BlurCache *cache, const gchar *pixbuf_hash, gdouble intensity);
```

### blur_cache_put()
```c
/**
 * Stores blur result in cache with LRU eviction
 * 
 * @param cache BlurCache instance
 * @param pixbuf_hash Hash of original pixbuf
 * @param intensity Blur intensity (rounded to 0.1 precision)
 * @param blurred_pixbuf Blur result to cache
 * @return TRUE if successfully cached, FALSE if rejected
 * 
 * @preconditions:
 * - cache != NULL
 * - pixbuf_hash is unique for source image
 * - blurred_pixbuf != NULL with valid dimensions
 * 
 * @postconditions:
 * - Item cached with current timestamp
 * - LRU eviction performed if necessary
 * - Reference count incremented for cached pixbuf
 * 
 * @memory_management:
 * - Takes ownership of blurred_pixbuf reference
 * - May evict older entries to stay within memory limits
 */
gboolean blur_cache_put(BlurCache *cache,
                       const gchar *pixbuf_hash,
                       gdouble intensity,
                       GdkPixbuf *blurred_pixbuf);
```

## Utility Functions API

### blur_calculate_sigma()
```c
/**
 * Calculates Gaussian sigma from intensity parameter
 * 
 * @param intensity Blur intensity 0.0-10.0
 * @return Sigma value for Gaussian kernel generation
 * 
 * @formula: sigma = intensity * 2.0
 * @range: [0.0, 20.0] corresponding to [0.0, 10.0] intensity
 */
gdouble blur_calculate_sigma(gdouble intensity);
```

### blur_calculate_kernel_size()
```c
/**
 * Calculates optimal kernel size for given sigma
 * 
 * @param sigma Gaussian sigma value
 * @return Odd kernel size covering 99.7% of distribution
 * 
 * @formula: size = 2 * ceil(3 * sigma) + 1
 * @constraints: Returns minimum 3, maximum 121
 */
gint blur_calculate_kernel_size(gdouble sigma);
```

### blur_generate_kernel()
```c
/**
 * Generates 1D Gaussian kernel for separable convolution
 * 
 * @param sigma Gaussian sigma parameter
 * @param kernel_size Size of kernel to generate (must be odd)
 * @return Dynamically allocated kernel array, caller must free()
 * 
 * @preconditions:
 * - sigma > 0.0
 * - kernel_size >= 3 && kernel_size is odd
 * 
 * @postconditions:
 * - Kernel values sum to 1.0 (normalized)
 * - Values follow Gaussian distribution
 * - Array has kernel_size elements
 */
gfloat* blur_generate_kernel(gdouble sigma, gint kernel_size);
```

## Error Handling Contracts

### Error Codes
```c
typedef enum {
    BLUR_ERROR_NONE = 0,
    BLUR_ERROR_INVALID_INTENSITY = 1,     // intensity not in [0.0, 10.0]
    BLUR_ERROR_INVALID_PIXBUF = 2,        // NULL or corrupted pixbuf
    BLUR_ERROR_MEMORY_ALLOCATION = 3,     // Failed to allocate memory
    BLUR_ERROR_PROCESSING_FAILED = 4,     // Blur algorithm failed
    BLUR_ERROR_THREAD_FAILED = 5,         // Thread creation/management failed
    BLUR_ERROR_CANCELLED = 6,             // Operation cancelled by user
    BLUR_ERROR_CACHE_FULL = 7            // Cache operation failed
} BlurErrorCode;
```

### Error Handling Guarantees
```c
/**
 * Error handling contract for all blur API functions:
 * 
 * SUCCESS CASE:
 * - Function returns expected value (non-NULL for pointers, TRUE for booleans)
 * - All output parameters populated with valid data
 * - Resource ownership transferred as documented
 * 
 * FAILURE CASE:  
 * - Function returns error indicator (NULL, FALSE, 0)
 * - No side effects on input parameters
 * - No partial resource allocation (all-or-nothing)
 * - Error details available through GError mechanism where applicable
 * 
 * THREAD SAFETY:
 * - All functions are thread-safe for concurrent access
 * - Processor instance can handle multiple simultaneous requests
 * - Cache instance uses internal locking for thread safety
 * 
 * MEMORY MANAGEMENT:
 * - All allocated resources have clear ownership semantics
 * - Reference counting used for shared pixbuf resources
 * - Automatic cleanup on processor/cache destruction
 */
```

## Performance Contracts

### Response Time Guarantees
```c
/**
 * Performance contracts for blur operations:
 * 
 * SYNCHRONOUS FUNCTIONS (immediate return):
 * - blur_processor_create(): <10ms
 * - blur_cache_get(): <1ms average, <5ms worst case
 * - blur_cache_put(): <5ms average, <20ms with eviction
 * - Utility functions: <0.1ms each
 * 
 * ASYNCHRONOUS FUNCTIONS (callback delivery):
 * - Progressive quality (is_progressive=TRUE):
 *   - 500x500 images: <15ms callback delivery
 *   - HD images (1920x1080): <50ms callback delivery
 *   - 4K images (3840x2160): <150ms callback delivery
 * 
 * - Full quality (is_progressive=FALSE):
 *   - 500x500 images: <25ms callback delivery  
 *   - HD images (1920x1080): <200ms callback delivery
 *   - 4K images (3840x2160): <500ms callback delivery
 * 
 * CANCELLATION:
 * - blur_processor_cancel(): <10ms acknowledgment
 * - Actual cancellation: <100ms from request
 */
```

### Memory Usage Contracts
```c
/**
 * Memory usage guarantees:
 * 
 * BLUR_PROCESSOR:
 * - Base overhead: <5MB for processor instance
 * - Per-request overhead: <2x source pixbuf size during processing
 * - Thread stack: <8MB total for worker thread pool
 * 
 * BLUR_CACHE:
 * - Overhead: <1MB for cache metadata
 * - Entry storage: Actual pixbuf size + ~100 bytes per entry
 * - Memory limit: Strictly enforced, never exceeded
 * 
 * TEMPORARY ALLOCATIONS:
 * - Kernel generation: <1KB per kernel (deallocated immediately)
 * - Processing buffers: 2x image size (deallocated after processing)
 * - No memory leaks under normal operation
 */
```

This API contract provides a complete interface specification for implementing the blur functionality with clear responsibilities, error handling, and performance guarantees.