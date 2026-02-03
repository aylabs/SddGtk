# HelloImageViewer Integration API Contracts

## UI Integration API

### hello_image_viewer_blur_init()
```c
/**
 * Initializes blur functionality in HelloImageViewer instance
 * 
 * @param viewer HelloImageViewer instance to extend
 * @param blur_processor Shared BlurProcessor for processing
 * @param blur_cache Shared BlurCache for caching results
 * @return TRUE on successful initialization, FALSE on failure
 * 
 * @preconditions:
 * - viewer != NULL && viewer is fully initialized
 * - blur_processor != NULL && processor is ready
 * - blur_cache != NULL && cache is ready
 * 
 * @postconditions:
 * - Blur UI widgets added to viewer's header bar
 * - Signal handlers connected for slider interaction
 * - Initial blur state set to 0.0 (no blur)
 * - Integration with existing B&W functionality established
 * 
 * @ui_elements_created:
 * - GtkScale for blur intensity selection
 * - GtkLabel for current intensity display
 * - GtkImage for blur icon
 * - GtkBox container for layout
 */
gboolean hello_image_viewer_blur_init(HelloImageViewer *viewer,
                                     BlurProcessor *blur_processor,
                                     BlurCache *blur_cache);
```

### hello_image_viewer_set_blur_intensity()
```c
/**
 * Programmatically sets blur intensity
 * 
 * @param viewer HelloImageViewer instance
 * @param intensity New blur intensity 0.0-10.0
 * @param animate TRUE to animate slider to new position
 * @return TRUE if intensity applied successfully, FALSE on error
 * 
 * @preconditions:
 * - viewer != NULL && blur is initialized
 * - 0.0 <= intensity <= 10.0
 * 
 * @postconditions:
 * - Slider updated to new position
 * - Value label updated to show new intensity
 * - Blur processing initiated if intensity > 0.0
 * - Image display updated when processing completes
 * 
 * @side_effects:
 * - May trigger blur processing and cache updates
 * - Cancels any in-progress blur operations
 * - Updates internal blur state
 */
gboolean hello_image_viewer_set_blur_intensity(HelloImageViewer *viewer,
                                              gdouble intensity,
                                              gboolean animate);
```

### hello_image_viewer_get_blur_intensity()
```c
/**
 * Gets current blur intensity setting
 * 
 * @param viewer HelloImageViewer instance  
 * @return Current blur intensity, or -1.0 if blur not initialized
 * 
 * @preconditions:
 * - viewer != NULL
 * 
 * @postconditions:
 * - Returns current slider position
 * - Value matches what's displayed in UI
 */
gdouble hello_image_viewer_get_blur_intensity(HelloImageViewer *viewer);
```

### hello_image_viewer_blur_reset()
```c
/**
 * Resets blur to disabled state (intensity = 0.0)
 * 
 * @param viewer HelloImageViewer instance
 * @param clear_cache TRUE to also clear cached blur results
 * @return TRUE on successful reset, FALSE on error
 * 
 * @preconditions:
 * - viewer != NULL && blur is initialized
 * 
 * @postconditions:
 * - Slider reset to 0.0 position
 * - Original image displayed (no blur)
 * - Processing cancelled if in progress
 * - Cache optionally cleared
 */
gboolean hello_image_viewer_blur_reset(HelloImageViewer *viewer, gboolean clear_cache);
```

## Signal Integration Contracts

### "blur-intensity-changed" Signal
```c
/**
 * Emitted when user changes blur intensity via slider
 * 
 * @param viewer HelloImageViewer instance emitting signal
 * @param intensity New blur intensity value 0.0-10.0
 * @param is_final TRUE if this is final value (drag ended), FALSE if dragging
 * @param user_data User data connected to signal
 * 
 * @emission_timing:
 * - During slider drag: is_final=FALSE, debounced to max 20Hz
 * - On drag end: is_final=TRUE, emitted once
 * - Programmatic changes: is_final=TRUE always
 * 
 * @use_cases:
 * - External components tracking blur state
 * - Persistence of blur settings
 * - Undo/redo functionality integration
 */
void (*blur_intensity_changed)(HelloImageViewer *viewer,
                              gdouble intensity,
                              gboolean is_final,
                              gpointer user_data);
```

### "blur-processing-started" Signal  
```c
/**
 * Emitted when blur processing begins
 * 
 * @param viewer HelloImageViewer instance
 * @param intensity Intensity being processed
 * @param is_progressive TRUE for progressive quality, FALSE for full
 * @param user_data User data connected to signal
 * 
 * @use_cases:
 * - Show progress indicators
 * - Disable UI elements during processing
 * - Update status bar with processing info
 */
void (*blur_processing_started)(HelloImageViewer *viewer,
                               gdouble intensity,
                               gboolean is_progressive,
                               gpointer user_data);
```

### "blur-processing-completed" Signal
```c
/**
 * Emitted when blur processing finishes (success or failure)
 * 
 * @param viewer HelloImageViewer instance
 * @param intensity Intensity that was processed
 * @param success TRUE if processing succeeded, FALSE on error
 * @param error_message Error description if success=FALSE, NULL otherwise
 * @param user_data User data connected to signal
 * 
 * @use_cases:
 * - Hide progress indicators
 * - Re-enable UI elements
 * - Display error messages to user
 * - Update status with completion time
 */
void (*blur_processing_completed)(HelloImageViewer *viewer,
                                 gdouble intensity,
                                 gboolean success,
                                 const gchar *error_message,
                                 gpointer user_data);
```

## Image Loading Integration

### hello_image_viewer_blur_on_image_loaded()
```c
/**
 * Called when new image is loaded into viewer
 * 
 * @param viewer HelloImageViewer instance
 * @param pixbuf New image pixbuf
 * @return TRUE if blur state updated successfully, FALSE on error
 * 
 * @preconditions:
 * - viewer != NULL && blur is initialized
 * - pixbuf != NULL && pixbuf is valid
 * 
 * @postconditions:
 * - Previous blur cache cleared for old image
 * - Blur processor updated with new image dimensions
 * - Current blur intensity preserved but re-applied to new image
 * - B&W conversion state considered in blur application
 * 
 * @integration_behavior:
 * - If blur intensity > 0.0: Apply blur to new image
 * - If B&W mode active: Apply B&W conversion after blur
 * - Cache key updated to reflect new image hash
 */
gboolean hello_image_viewer_blur_on_image_loaded(HelloImageViewer *viewer, GdkPixbuf *pixbuf);
```

### hello_image_viewer_blur_on_image_unloaded()
```c
/**
 * Called when image is unloaded from viewer
 * 
 * @param viewer HelloImageViewer instance
 * 
 * @preconditions:
 * - viewer != NULL
 * 
 * @postconditions:
 * - All processing cancelled
 * - Image-specific cache entries cleared
 * - Blur UI remains available but inactive
 * - Slider retains current position for next image
 */
void hello_image_viewer_blur_on_image_unloaded(HelloImageViewer *viewer);
```

## B&W Integration Contracts

### hello_image_viewer_blur_bw_compatibility()
```c
/**
 * Ensures proper integration between blur and B&W effects
 * 
 * @param viewer HelloImageViewer instance
 * @param bw_mode_active TRUE if B&W conversion should be applied
 * @return TRUE if effects applied successfully, FALSE on error
 * 
 * @effect_order:
 * 1. Start with original color image
 * 2. Apply blur if intensity > 0.0
 * 3. Apply B&W conversion if bw_mode_active=TRUE
 * 4. Display final processed image
 * 
 * @caching_strategy:
 * - Cache both blurred-color and blurred-bw versions
 * - Use separate cache keys for color vs B&W results
 * - Reuse blurred result when toggling B&W mode
 * 
 * @performance:
 * - B&W toggle on blurred image: <50ms
 * - Blur change with B&W active: Apply blur then B&W conversion
 */
gboolean hello_image_viewer_blur_bw_compatibility(HelloImageViewer *viewer, gboolean bw_mode_active);
```

## Configuration and Preferences

### hello_image_viewer_blur_get_config()
```c
/**
 * Retrieves current blur configuration settings
 * 
 * @param viewer HelloImageViewer instance
 * @return BlurConfig struct with current settings, or NULL on error
 * 
 * @config_includes:
 * - Current intensity value
 * - Cache size and memory limits
 * - Progressive quality preferences
 * - UI element visibility settings
 */
BlurConfig* hello_image_viewer_blur_get_config(HelloImageViewer *viewer);
```

### hello_image_viewer_blur_set_config()
```c
/**
 * Applies blur configuration settings
 * 
 * @param viewer HelloImageViewer instance
 * @param config BlurConfig with new settings
 * @return TRUE if configuration applied successfully, FALSE on error
 * 
 * @preconditions:
 * - viewer != NULL && blur initialized
 * - config != NULL && config is valid
 * 
 * @postconditions:
 * - Settings applied immediately where possible
 * - Cache resized if memory limits changed
 * - UI updated to reflect new configuration
 * - Changes persist until next configuration update
 */
gboolean hello_image_viewer_blur_set_config(HelloImageViewer *viewer, const BlurConfig *config);
```

## State Management Contracts

### BlurState Enumeration
```c
typedef enum {
    HELLO_IMAGE_VIEWER_BLUR_DISABLED,    // intensity = 0.0, no processing
    HELLO_IMAGE_VIEWER_BLUR_READY,       // intensity > 0.0, no processing active
    HELLO_IMAGE_VIEWER_BLUR_PROCESSING,  // background blur in progress
    HELLO_IMAGE_VIEWER_BLUR_COMPLETE,    // processing done, blurred image shown
    HELLO_IMAGE_VIEWER_BLUR_ERROR       // processing failed, original image shown
} HelloImageViewerBlurState;
```

### hello_image_viewer_get_blur_state()
```c
/**
 * Gets current blur processing state
 * 
 * @param viewer HelloImageViewer instance
 * @return Current blur state, or BLUR_ERROR if viewer invalid
 * 
 * @state_transitions:
 * - DISABLED → READY: When intensity changed from 0.0 to >0.0
 * - READY → PROCESSING: When blur operation starts
 * - PROCESSING → COMPLETE: When blur operation succeeds  
 * - PROCESSING → ERROR: When blur operation fails
 * - COMPLETE → READY: When intensity changed
 * - ERROR → READY: When intensity changed (retry)
 * - Any → DISABLED: When intensity set to 0.0
 */
HelloImageViewerBlurState hello_image_viewer_get_blur_state(HelloImageViewer *viewer);
```

## Memory and Resource Management

### Resource Cleanup Contract
```c
/**
 * Resource management guarantees:
 * 
 * AUTOMATIC CLEANUP:
 * - Blur resources cleaned up when HelloImageViewer destroyed
 * - Processing cancelled automatically on viewer destruction
 * - Cache cleared automatically on image changes
 * - Temporary processing buffers freed after each operation
 * 
 * MANUAL CLEANUP:
 * - hello_image_viewer_blur_reset() clears processing state
 * - Cache can be manually cleared via configuration
 * - Individual cache entries evicted based on LRU policy
 * 
 * THREAD SAFETY:
 * - All blur functions safe to call from main thread
 * - Background processing properly synchronized
 * - Cache access protected by internal locking
 * - No race conditions in resource cleanup
 * 
 * ERROR RECOVERY:
 * - Failed operations don't leak resources
 * - Partial processing results discarded on error
 * - Cache remains consistent even after errors
 * - UI state restored to consistent state on errors
 */
```

This integration API contract defines how the blur functionality integrates seamlessly with the existing HelloImageViewer while maintaining clean separation of concerns and robust error handling.