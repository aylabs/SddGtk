# API Contracts: Image B&W Conversion

**Feature**: Image Black & White Conversion  
**Created**: February 3, 2026  
**Purpose**: Define interfaces and contracts for B&W conversion functionality

## Image Processing Library Contract

### Core Conversion Interface

```c
/**
 * image_processor_convert_to_grayscale:
 * @original: Source color image (must be valid GdkPixbuf)
 * @error: Location to store error information (nullable)
 * 
 * Converts color image to grayscale using ITU-R BT.709 luminance formula.
 * Original pixbuf is never modified. Returns new pixbuf or NULL on error.
 * 
 * Returns: (transfer full): New grayscale GdkPixbuf or NULL
 */
GdkPixbuf* image_processor_convert_to_grayscale(GdkPixbuf *original, GError **error);

/**
 * image_processor_validate_pixbuf:
 * @pixbuf: GdkPixbuf to validate (nullable)
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
```

**Contract Requirements**:
- Function must be thread-safe for future async processing
- Must handle all GdkPixbuf supported formats (PNG, JPEG, GIF, SVG, WebP)
- Must preserve exact pixel dimensions and transparency
- Must use luminance-based conversion: `Y = 0.299*R + 0.587*G + 0.114*B`
- Must return NULL and set GError for all failure conditions
- Memory management: caller owns returned pixbuf (transfer full)

### Error Domain Contract

```c
#define IMAGE_PROCESSOR_ERROR (image_processor_error_quark())

typedef enum {
    IMAGE_PROCESSOR_ERROR_INVALID_INPUT,     /* NULL or invalid pixbuf */
    IMAGE_PROCESSOR_ERROR_MEMORY_ALLOCATION, /* Insufficient memory */
    IMAGE_PROCESSOR_ERROR_CONVERSION_FAILED, /* Processing failure */
    IMAGE_PROCESSOR_ERROR_UNSUPPORTED_FORMAT /* Format not supported */
} ImageProcessorError;

GQuark image_processor_error_quark(void);
```

**Error Handling Contract**:
- All functions must set detailed error information on failure
- Error messages must be user-friendly and actionable
- Must distinguish between temporary failures (memory) and permanent failures (format)
- Error domain must integrate with existing GTK error handling patterns

## HelloImageViewer Extension Contract

### Public Interface Extensions

```c
/**
 * hello_image_viewer_set_conversion_enabled:
 * @viewer: A HelloImageViewer instance
 * @enabled: Whether B&W conversion feature is available
 * 
 * Enables or disables the B&W conversion button.
 * When disabled, button is hidden and conversion state is reset.
 */
void hello_image_viewer_set_conversion_enabled(HelloImageViewer *viewer, gboolean enabled);

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
 * hello_image_viewer_toggle_conversion:
 * @viewer: A HelloImageViewer instance
 * 
 * Programmatically toggles between color and B&W modes.
 * Equivalent to user clicking the conversion button.
 * 
 * Returns: TRUE if toggle was successful
 */
gboolean hello_image_viewer_toggle_conversion(HelloImageViewer *viewer);
```

### Signal Interface Contract

```c
/**
 * HelloImageViewer::conversion-state-changed:
 * @viewer: The HelloImageViewer instance  
 * @is_grayscale: New conversion state (TRUE = B&W, FALSE = color)
 * 
 * Emitted when image conversion state changes.
 * Useful for external UI synchronization or state tracking.
 */
void (*conversion_state_changed) (HelloImageViewer *viewer, gboolean is_grayscale);

/**
 * HelloImageViewer::conversion-error:
 * @viewer: The HelloImageViewer instance
 * @error: GError containing failure details
 * 
 * Emitted when image conversion fails.
 * Applications should display error to user and reset UI state.
 */
void (*conversion_error) (HelloImageViewer *viewer, GError *error);
```

**Signal Contract Requirements**:
- Signals must be emitted only when state actually changes
- Error signals must provide actionable error information
- Signal emission must not block UI thread
- Must be compatible with existing GObject signal patterns

### Property Interface Contract

```c
enum {
    PROP_CONVERSION_ENABLED = 1,
    PROP_CONVERSION_STATE,
    PROP_ORIGINAL_PIXBUF,
    N_PROPS
};

/**
 * HelloImageViewer:conversion-enabled:
 * 
 * Whether the B&W conversion feature is enabled.
 * When FALSE, conversion button is hidden.
 * 
 * Default: TRUE
 */
gboolean conversion_enabled;

/**
 * HelloImageViewer:conversion-state:
 * 
 * Current image conversion state.
 * TRUE = grayscale, FALSE = original colors.
 * Read-only property that reflects current display state.
 * 
 * Default: FALSE
 */
gboolean conversion_state;
```

**Property Contract Requirements**:
- Properties must be bindable for UI framework integration
- Property changes must emit notify signals for observers
- Must maintain consistency with internal widget state
- Must be compatible with existing HelloImageViewer properties

## UI Template Contract

### GtkBuilder Template Requirements

```xml
<!-- Required button structure in hello-image-viewer.ui -->
<object class="GtkToggleButton" id="conversion_button">
    <property name="icon-name">image-filter-symbolic</property>
    <property name="tooltip-text" translatable="yes">Convert to black and white</property>
    <property name="sensitive">false</property>
    <property name="action-name">win.toggle-conversion</property>
    <signal name="toggled" handler="on_conversion_button_toggled" swapped="no"/>
</object>
```

**Template Contract**:
- Button must be GtkToggleButton for proper state indication
- Must use standard symbolic icons for theme compatibility
- Must include translatable tooltip text for internationalization
- Must be initially disabled (enabled when image loads)
- Must connect to proper signal handler in HelloImageViewer
- Must support keyboard navigation and accessibility

### CSS Styling Contract

```css
/* Required CSS classes for conversion button states */
.image-viewer-button {
    margin: 6px;
    border-radius: 6px;
}

.image-viewer-button:checked {
    background: @accent_color;
    color: @accent_fg_color;
}

.image-viewer-button.processing {
    opacity: 0.7;
}
```

**Styling Contract Requirements**:
- Must provide visual feedback for button states
- Must be compatible with GTK4 theming system
- Must support high contrast and accessibility themes
- Must maintain consistency with existing application styling

## Memory Management Contract

### Image Data Lifecycle

```c
/**
 * Image memory management contract:
 * 
 * 1. Original pixbuf: Retained for entire viewer session
 * 2. Converted pixbuf: Created/destroyed on each conversion
 * 3. Display pixbuf: Points to either original or converted
 * 4. Memory limit: Configurable maximum for image cache
 */

typedef struct {
    GdkPixbuf *original_pixbuf;    /* Never freed during session */
    GdkPixbuf *converted_pixbuf;   /* Created/freed as needed */
    GdkPixbuf *display_pixbuf;     /* Pointer to current display */
    gboolean is_converted;         /* State flag */
    gsize memory_usage;            /* Current memory footprint */
} ImageConversionData;
```

**Memory Contract Requirements**:
- Original image data must never be modified or freed during session
- Converted image data must be freed when no longer needed
- Memory usage must be tracked and bounded
- Must handle low memory conditions gracefully
- Must integrate with GObject reference counting system

### Performance Requirements

**Conversion Performance Contract**:
- Images up to 4K resolution: < 1 second conversion time
- Images up to 50MB file size: Must complete without system impact
- Memory usage: Maximum 3x original image size during conversion
- UI responsiveness: Conversion must not block main thread for > 100ms
- Multiple windows: Each window conversion operates independently

**Quality Assurance Contract**:
- Conversion must be lossless and reversible
- No visual artifacts introduced during conversion
- Exact pixel dimensions preserved
- Transparency channel preserved for supported formats
- Color profile information maintained where applicable

## Testing Interface Contract

### Unit Test Requirements

```c
/**
 * Required test coverage for image processing functions:
 */

// Core conversion tests
void test_convert_to_grayscale_valid_input(void);
void test_convert_to_grayscale_null_input(void);
void test_convert_to_grayscale_large_image(void);

// Validation tests  
void test_validate_pixbuf_valid_cases(void);
void test_validate_pixbuf_invalid_cases(void);

// Memory estimation tests
void test_estimate_memory_usage_accuracy(void);
void test_estimate_memory_usage_edge_cases(void);

// Integration tests
void test_viewer_conversion_toggle(void);
void test_viewer_multiple_windows(void);
void test_viewer_error_handling(void);
```

**Test Contract Requirements**:
- Must achieve 100% code coverage for conversion algorithms
- Must test all error conditions and edge cases
- Must validate memory management and leak detection
- Must test cross-platform behavior consistency
- Must include performance regression testing

### Integration Test Contract

```python
"""
Required DoGTail integration tests for UI behavior:
"""

def test_conversion_button_availability():
    """Button enabled only when image loaded"""
    pass

def test_conversion_toggle_behavior():
    """Button state changes with image conversion"""
    pass

def test_conversion_visual_feedback():
    """UI provides clear state indication"""
    pass

def test_multiple_window_independence():
    """Each window maintains independent state"""
    pass

def test_accessibility_compliance():
    """Full keyboard and screen reader support"""
    pass
```

**Integration Test Requirements**:
- Must verify complete user workflows
- Must test accessibility features comprehensively  
- Must validate cross-window behavior independence
- Must confirm error handling UI feedback
- Must test internationalization and theming

## Platform Compatibility Contract

### Cross-Platform Requirements

**Linux Platform**:
- GTK4 version 4.0+ with native GdkPixbuf support
- Standard symbolic icons available in icon theme
- Integration with system accessibility services

**Windows Platform (MSYS2)**:
- MinGW-w64 GTK4 packages with full GdkPixbuf support
- Windows-specific icon theme compatibility
- Integration with Windows accessibility APIs

**macOS Platform (Homebrew)**:
- Homebrew GTK4 with Quartz backend
- macOS icon theme integration
- VoiceOver accessibility support

**Universal Requirements**:
- Identical conversion algorithms across all platforms
- Consistent visual appearance and behavior
- Platform-specific accessibility integration
- Performance parity within 10% variance

## Deployment Contract

### Build System Integration

```meson
# Required meson.build additions
image_processing_lib = static_library('image-processing',
  'src/lib/image-processing.c',
  dependencies: [gtk_dep],
  include_directories: inc
)

# Updated executable dependencies
hello_app = executable('hello-app',
  sources: [..., 'src/hello-app/hello-image-viewer.c'],
  dependencies: [gtk_dep],
  link_with: [gtk_utils_lib, image_processing_lib],
  install: true
)
```

**Build Contract Requirements**:
- New image processing library must be static linked
- Must maintain existing build configuration compatibility
- Must support both Meson and CMake build systems
- Must include proper dependency declarations
- Must support cross-compilation for all target platforms