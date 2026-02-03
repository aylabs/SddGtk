# Quickstart: Implementing Image Blur Effect

This guide provides developers with everything needed to implement the blur effect feature in the GTK4 Image Viewer application.

## Overview

The blur effect adds a Gaussian blur slider to the image viewer, allowing real-time intensity adjustment from 0.0 (no blur) to 10.0 (maximum blur). The implementation uses separable Gaussian convolution for optimal performance and includes caching for frequently used blur intensities.

## Prerequisites

- **Development Environment**: Complete GTK4 Image Viewer project setup
- **Dependencies**: GTK4 4.20.3+, GdkPixbuf, GThread support  
- **Knowledge Required**: C11, GTK4 widget development, basic image processing concepts
- **Estimated Implementation Time**: 2-3 days for experienced GTK developers

## Quick Implementation Checklist

### Phase 1: Core Blur Library (Day 1 Morning)
- [ ] Create `src/lib/blur-processor.h` with API definitions
- [ ] Implement `blur_processor_create()` and resource management  
- [ ] Implement Gaussian kernel generation functions
- [ ] Add separable convolution algorithm with threading support
- [ ] Create comprehensive unit tests for blur algorithms

### Phase 2: Cache Implementation (Day 1 Afternoon)  
- [ ] Create `src/lib/blur-cache.h` with LRU cache interface
- [ ] Implement hash-based cache with memory limits
- [ ] Add LRU eviction with reference counting
- [ ] Create cache performance tests

### Phase 3: UI Integration (Day 2 Morning)
- [ ] Extend `hello-image-viewer.ui` template with blur widgets
- [ ] Add blur slider, icon, and value label to header bar
- [ ] Implement signal handlers for real-time slider updates
- [ ] Add debouncing and progressive quality rendering

### Phase 4: Feature Integration (Day 2 Afternoon)
- [ ] Integrate blur with existing B&W conversion feature
- [ ] Add blur state to HelloImageViewer private struct  
- [ ] Implement image loading/unloading handlers
- [ ] Add configuration persistence support

### Phase 5: Testing & Polish (Day 3)
- [ ] Create integration tests with real images
- [ ] Performance testing and optimization  
- [ ] Error handling and edge case validation
- [ ] Documentation and code review

## Key Implementation Files

### Core Library Files
```
src/lib/blur-processor.h         # Main blur processing API
src/lib/blur-processor.c         # Separable Gaussian implementation
src/lib/blur-cache.h             # LRU cache interface  
src/lib/blur-cache.c             # Cache implementation with threading
```

### UI Integration Files
```
src/hello-app/hello-image-viewer.h    # Extended with blur support
src/hello-app/hello-image-viewer.c    # Blur UI and signal handling
src/hello-app/resources/hello-image-viewer.ui  # Blur widgets in template
src/hello-app/resources/image-viewer.css       # Blur widget styling
```

### Testing Files
```
tests/unit/test-blur-processor.c      # Core algorithm tests
tests/unit/test-blur-cache.c          # Cache functionality tests
tests/integration/test-blur-ui.c      # UI integration tests
tests/performance/blur-benchmarks.c   # Performance validation
```

## Code Examples

### Basic Blur Processing
```c
// Initialize blur processor
BlurProcessor *processor = blur_processor_create(1920, 1080, 0);

// Apply blur asynchronously
guint request_id = blur_processor_apply_async(
    processor,
    original_pixbuf,
    5.0,  // intensity
    FALSE, // full quality
    on_blur_complete,
    user_data
);

// Completion callback
static void on_blur_complete(GdkPixbuf *result, const GError *error, gpointer user_data) {
    if (error) {
        g_warning("Blur failed: %s", error->message);
        return;
    }
    
    // Update UI with blurred result
    gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), result);
    g_object_unref(result);
}
```

### UI Slider Integration
```c
// In hello_image_viewer_init()
GtkWidget *blur_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 10.0, 0.1);
gtk_scale_set_digits(GTK_SCALE(blur_scale), 1);
gtk_widget_set_size_request(blur_scale, 150, -1);

// Add marks for visual reference
gtk_scale_add_mark(GTK_SCALE(blur_scale), 0.0, GTK_POS_BOTTOM, "None");
gtk_scale_add_mark(GTK_SCALE(blur_scale), 5.0, GTK_POS_BOTTOM, "Med");
gtk_scale_add_mark(GTK_SCALE(blur_scale), 10.0, GTK_POS_BOTTOM, "Max");

// Connect signal with debouncing
g_signal_connect(blur_scale, "value-changed", G_CALLBACK(on_blur_intensity_changed), viewer);
```

### Cache Integration
```c
// Check cache before processing
gchar *cache_key = g_strdup_printf("%.1f", blur_intensity);
GdkPixbuf *cached_result = blur_cache_get(cache, image_hash, blur_intensity);

if (cached_result) {
    // Use cached result immediately
    update_image_display(viewer, cached_result);
    g_object_unref(cached_result);
} else {
    // Start background processing
    blur_processor_apply_async(processor, original_pixbuf, blur_intensity, 
                              is_progressive, on_blur_complete, viewer);
}
```

## Performance Guidelines

### Real-time Responsiveness
- **Target**: <100ms UI response for slider updates
- **Implementation**: Use progressive quality during drag operations
- **Fallback**: Show original image if processing takes >500ms

### Memory Management  
- **Cache Limit**: 150MB maximum for blur result cache
- **Processing Overhead**: <2x source image size during processing
- **Cleanup**: Automatic cache eviction using LRU policy

### Threading Strategy
```c
// Use background threads for images larger than 1000x1000
gint width = gdk_pixbuf_get_width(pixbuf);
gint height = gdk_pixbuf_get_height(pixbuf);
gboolean use_background = (width * height > 1000000);

if (use_background) {
    blur_processor_apply_async(processor, pixbuf, intensity, FALSE, callback, data);
} else {
    // Small image - process synchronously
    GdkPixbuf *result = blur_processor_apply_sync(processor, pixbuf, intensity);
    callback(result, NULL, data);
}
```

## Testing Strategy

### Unit Testing
```bash
# Run blur algorithm tests
./build/tests/unit/test-blur-processor

# Run cache functionality tests  
./build/tests/unit/test-blur-cache

# Run integration tests
./build/tests/integration/test-blur-ui
```

### Performance Validation
```bash
# Benchmark blur performance across image sizes
./build/tests/performance/blur-benchmarks

# Expected results:
# 500x500:     <25ms   (✓ Real-time capable)
# 1920x1080:   <200ms  (✓ Acceptable for full quality)
# 3840x2160:   <500ms  (✓ Meets requirements)
```

### Memory Testing
```bash
# Monitor memory usage during blur operations
valgrind --tool=massif ./build/src/hello-app/image-viewer

# Verify no memory leaks
valgrind --tool=memcheck --leak-check=full ./build/src/hello-app/image-viewer
```

## Common Implementation Pitfalls

### Threading Issues
❌ **Don't**: Access GTK widgets from background threads
```c
// WRONG - GTK widgets not thread-safe
static void worker_thread_func(gpointer data) {
    gtk_image_set_from_pixbuf(image, result);  // ❌ Thread safety violation
}
```

✅ **Do**: Use g_idle_add for UI updates from background threads
```c
typedef struct {
    GtkImage *image;
    GdkPixbuf *result;
} UIUpdateData;

static gboolean update_ui_idle(gpointer data) {
    UIUpdateData *update_data = (UIUpdateData*)data;
    gtk_image_set_from_pixbuf(update_data->image, update_data->result);
    
    g_object_unref(update_data->result);
    g_free(update_data);
    return G_SOURCE_REMOVE;
}

static void worker_thread_func(gpointer data) {
    // ... blur processing ...
    
    UIUpdateData *update_data = g_new(UIUpdateData, 1);
    update_data->image = image;
    update_data->result = g_object_ref(result);
    
    g_idle_add(update_ui_idle, update_data);  // ✅ Thread-safe UI update
}
```

### Memory Management
❌ **Don't**: Forget to handle pixbuf reference counting
```c
GdkPixbuf *result = blur_processor_apply_sync(processor, pixbuf, intensity);
gtk_image_set_from_pixbuf(image, result);
// ❌ Memory leak - result pixbuf never unreferenced
```

✅ **Do**: Properly manage pixbuf references
```c
GdkPixbuf *result = blur_processor_apply_sync(processor, pixbuf, intensity);
gtk_image_set_from_pixbuf(image, result);
g_object_unref(result);  // ✅ Release reference after use
```

### Performance Issues  
❌ **Don't**: Process every slider movement immediately
```c
static void on_blur_scale_changed(GtkScale *scale, gpointer user_data) {
    gdouble intensity = gtk_range_get_value(GTK_RANGE(scale));
    apply_blur_immediately(intensity);  // ❌ Too frequent, poor performance
}
```

✅ **Do**: Use debouncing and progressive quality
```c
static guint blur_timeout_id = 0;

static gboolean apply_blur_delayed(gpointer user_data) {
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(user_data);
    gdouble intensity = gtk_range_get_value(GTK_RANGE(viewer->blur_scale));
    
    // Apply full quality blur
    apply_blur_with_quality(viewer, intensity, FALSE);
    
    blur_timeout_id = 0;
    return G_SOURCE_REMOVE;
}

static void on_blur_scale_changed(GtkScale *scale, gpointer user_data) {
    HelloImageViewer *viewer = HELLO_IMAGE_VIEWER(user_data);
    gdouble intensity = gtk_range_get_value(GTK_RANGE(scale));
    
    // Apply progressive quality immediately for responsiveness  
    apply_blur_with_quality(viewer, intensity, TRUE);
    
    // Schedule full quality update with debouncing
    if (blur_timeout_id > 0) {
        g_source_remove(blur_timeout_id);
    }
    blur_timeout_id = g_timeout_add(250, apply_blur_delayed, viewer);  // ✅ Debounced
}
```

## Build Integration

### Meson Build Updates
Add to `src/lib/meson.build`:
```meson
lib_sources += files(
  'blur-processor.c',
  'blur-cache.c',
)

lib_headers += files(
  'blur-processor.h', 
  'blur-cache.h',
)
```

Add to `tests/meson.build`:
```meson
test('blur-processor', executable('test-blur-processor',
  'unit/test-blur-processor.c',
  dependencies: [gtk_dep, lib_dep],
))

test('blur-cache', executable('test-blur-cache', 
  'unit/test-blur-cache.c',
  dependencies: [gtk_dep, lib_dep],
))
```

## Debugging Tips

### Enable Blur Debug Logging
```c
// Add to blur-processor.c for detailed logging
#define BLUR_DEBUG 1

#if BLUR_DEBUG
#define blur_debug(fmt, ...) g_debug("BLUR: " fmt, ##__VA_ARGS__)
#else  
#define blur_debug(fmt, ...) 
#endif

// Usage in blur functions
blur_debug("Processing %dx%d image with intensity %.1f", width, height, intensity);
blur_debug("Cache hit for intensity %.1f", intensity);
blur_debug("Background thread processing completed in %ldms", processing_time);
```

### Visual Cache Statistics
```c
// Add cache statistics display in debug builds
#ifdef DEBUG
static void print_cache_stats(BlurCache *cache) {
    g_print("Cache Stats:\n");
    g_print("  Entries: %u/%u\n", cache->current_entries, cache->max_entries);
    g_print("  Memory: %.1fMB/%.1fMB\n", 
           cache->current_memory / (1024.0*1024.0),
           cache->max_memory / (1024.0*1024.0));
    g_print("  Hit Rate: %.1f%%\n", cache->hit_rate * 100.0);
}
#endif
```

## Ready to Start?

1. **Set up development branch**: `git checkout -b feature/blur-effect`
2. **Copy header templates**: Use the API contracts as starting points for .h files
3. **Start with blur-processor.c**: Implement core Gaussian blur algorithm first
4. **Add unit tests early**: Test each component as you build it
5. **Integrate incrementally**: Add UI components after core functionality works

The implementation should take approximately 2-3 days following this guide. Focus on getting the core blur algorithm working first, then add the UI integration and performance optimizations.