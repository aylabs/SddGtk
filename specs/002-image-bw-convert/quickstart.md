# Quick Start Guide: Image B&W Conversion Feature

**Feature**: Image Black & White Conversion Toggle  
**Target Audience**: Developers implementing the B&W conversion functionality  
**Prerequisites**: Existing HelloImageViewer implementation and GTK4 development environment

## Feature Overview

Add a toggle button to the HelloImageViewer window that converts displayed images between color and grayscale (black & white) modes. The feature preserves original image data for instant restoration and provides clear visual feedback through button state changes.

## Development Environment Setup

### Prerequisites Verification

```bash
# Verify existing GTK4 development setup
pkg-config --modversion gtk4  # Should show 4.0+
ls src/hello-app/hello-image-viewer.{c,h}  # Should exist from previous implementation

# Check that current image viewer builds and runs
cd build
meson compile
./hello-app
```

### Additional Dependencies

No new system dependencies required. The feature uses existing GTK4 and GdkPixbuf libraries already in the project.

## Implementation Steps

### Step 1: Create Image Processing Library

**Create**: `src/lib/image-processing.{c,h}`

**Key Functions**:
```c
// Primary conversion function
GdkPixbuf* image_processor_convert_to_grayscale(GdkPixbuf *original, GError **error);

// Validation function  
gboolean image_processor_validate_pixbuf(GdkPixbuf *pixbuf);

// Memory estimation
gsize image_processor_estimate_memory_usage(gint width, gint height);
```

**Core Algorithm** (ITU-R BT.709 standard):
```c
// For each pixel: Y = 0.299*R + 0.587*G + 0.114*B
guchar gray_value = (guchar)(0.299 * red + 0.587 * green + 0.114 * blue);
```

### Step 2: Extend HelloImageViewer Class

**Modify**: `src/hello-app/hello-image-viewer.{c,h}`

**Add Private Members**:
```c
struct _HelloImageViewer {
    GtkWindow parent_instance;
    
    // Existing members
    GtkWidget *image_widget;
    char *current_filename;
    
    // New members for B&W conversion
    GtkWidget *conversion_button;
    GdkPixbuf *original_pixbuf;     // Never modified, session-persistent
    GdkPixbuf *converted_pixbuf;    // Created/freed as needed
    gboolean is_converted;          // Current state flag
};
```

**Add Public Methods**:
```c
// Toggle conversion state
gboolean hello_image_viewer_toggle_conversion(HelloImageViewer *viewer);

// Get/set conversion state
gboolean hello_image_viewer_get_conversion_state(HelloImageViewer *viewer);
void hello_image_viewer_set_conversion_enabled(HelloImageViewer *viewer, gboolean enabled);
```

### Step 3: Create UI Template

**Create**: `src/hello-app/resources/hello-image-viewer.ui`

**Template Structure**:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <template class="HelloImageViewer" parent="GtkWindow">
    <property name="title">Image Viewer</property>
    <property name="default-width">600</property>
    <property name="default-height">400</property>
    <child>
      <object class="GtkBox" id="main_container">
        <property name="orientation">vertical</property>
        
        <!-- Header bar with conversion button -->
        <child>
          <object class="GtkHeaderBar" id="header_bar">
            <property name="show-title-buttons">true</property>
            <child type="start">
              <object class="GtkToggleButton" id="conversion_button">
                <property name="icon-name">image-filter-symbolic</property>
                <property name="tooltip-text" translatable="yes">Convert to black and white</property>
                <property name="sensitive">false</property>
                <signal name="toggled" handler="on_conversion_button_toggled"/>
              </object>
            </child>
          </object>
        </child>
        
        <!-- Image display area -->
        <child>
          <object class="GtkScrolledWindow" id="scrolled_window">
            <property name="hexpand">true</property>
            <property name="vexpand">true</property>
            <property name="hscrollbar-policy">automatic</property>
            <property name="vscrollbar-policy">automatic</property>
            <child>
              <object class="GtkPicture" id="image_widget">
                <property name="content-fit">contain</property>
                <property name="can-shrink">true</property>
              </object>
            </child>
          </object>
        </child>
        
      </object>
    </child>
  </template>
</interface>
```

### Step 4: Implement Signal Handlers

**Core Handler Implementation**:
```c
static void
on_conversion_button_toggled(GtkToggleButton *button, HelloImageViewer *viewer)
{
    GError *error = NULL;
    gboolean is_active = gtk_toggle_button_get_active(button);
    
    if (is_active) {
        // Convert to grayscale
        if (!viewer->converted_pixbuf) {
            viewer->converted_pixbuf = image_processor_convert_to_grayscale(
                viewer->original_pixbuf, &error);
                
            if (!viewer->converted_pixbuf) {
                // Handle conversion error
                gtk_toggle_button_set_active(button, FALSE);
                show_error_dialog(viewer, error);
                g_error_free(error);
                return;
            }
        }
        
        // Update display
        gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), 
                              viewer->converted_pixbuf);
        viewer->is_converted = TRUE;
        
        // Update button appearance
        gtk_button_set_icon_name(GTK_BUTTON(button), "image-restore-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Restore original colors");
        
    } else {
        // Restore original
        gtk_picture_set_pixbuf(GTK_PICTURE(viewer->image_widget), 
                              viewer->original_pixbuf);
        viewer->is_converted = FALSE;
        
        // Update button appearance
        gtk_button_set_icon_name(GTK_BUTTON(button), "image-filter-symbolic");
        gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Convert to black and white");
    }
}
```

### Step 5: Update Build Configuration

**Modify**: `meson.build`

```meson
# Add image processing library
image_processing_lib = static_library('image-processing',
  'src/lib/image-processing.c',
  dependencies: [gtk_dep],
  include_directories: inc
)

# Update main executable dependencies
hello_app = executable('hello-app',
  [
    'src/hello-app/main.c',
    'src/hello-app/hello-application.c',
    'src/hello-app/hello-window.c',
    'src/hello-app/hello-image-viewer.c',
    resources,
  ],
  dependencies: [gtk_dep],
  link_with: [gtk_utils_lib, image_processing_lib],  # Add new library
  include_directories: inc,
  install: true
)
```

**Update Resource Bundle** (`src/hello-app/hello-app.gresource.xml`):
```xml
<gresources>
  <gresource prefix="/com/example/HelloApp">
    <file>resources/hello-window.ui</file>
    <file>resources/hello-image-viewer.ui</file>  <!-- Add new template -->
  </gresource>
</gresources>
```

## Development Workflow

### Build and Test

```bash
# Clean and rebuild
cd build
meson compile

# Test basic functionality
./hello-app
# 1. Click "Open Image" button
# 2. Select a color image (PNG, JPEG, etc.)
# 3. Image viewer window opens with conversion button
# 4. Click conversion button - image should turn grayscale
# 5. Click again - image should restore to color
```

### Debug and Validation

**Memory Usage Debugging**:
```bash
# Run with memory validation
valgrind --tool=memcheck --leak-check=full ./hello-app

# Check for memory leaks during conversion operations
# Expected: No leaks, proper pixbuf cleanup
```

**Visual Validation**:
```bash
# Test with GTK Inspector for UI debugging
GTK_DEBUG=interactive ./hello-app

# Verify:
# - Button state changes correctly
# - Icons update properly
# - Tooltips change appropriately  
# - Accessibility properties are correct
```

### Performance Testing

**Large Image Testing**:
```bash
# Test with various image sizes
# Small (< 1MB): Instant conversion
# Medium (1-10MB): < 1 second
# Large (10-50MB): < 5 seconds, no UI blocking
```

**Multi-Window Testing**:
```bash
# Open multiple image viewers
# Convert some to B&W, leave others in color
# Verify independent state management
# Check memory usage with multiple images loaded
```

## Common Issues and Solutions

### Issue 1: Button Not Enabled
**Symptom**: Conversion button remains disabled even with image loaded  
**Solution**: Verify `hello_image_viewer_load_image()` enables the button:
```c
// In image load success path
gtk_widget_set_sensitive(viewer->conversion_button, TRUE);
```

### Issue 2: Conversion Takes Too Long
**Symptom**: UI freezes during conversion  
**Solution**: Check image size and add progress feedback for large images:
```c
// Before conversion
if (image_processor_estimate_memory_usage(width, height) > LARGE_IMAGE_THRESHOLD) {
    show_progress_dialog();
}
```

### Issue 3: Memory Leaks
**Symptom**: Memory usage increases with repeated conversions  
**Solution**: Ensure proper pixbuf cleanup:
```c
// In dispose method
if (viewer->converted_pixbuf) {
    g_object_unref(viewer->converted_pixbuf);
    viewer->converted_pixbuf = NULL;
}
```

### Issue 4: Icons Not Displaying
**Symptom**: Button shows text instead of icons  
**Solution**: Verify icon theme has required symbols:
```bash
# Check available icons
gtk4-icon-browser | grep filter
# Should show: image-filter-symbolic, image-restore-symbolic
```

## Testing Checklist

**Functional Testing**:
- [ ] Button enables when image loads
- [ ] First click converts to grayscale
- [ ] Second click restores color
- [ ] Button icon and tooltip update correctly
- [ ] Multiple windows work independently
- [ ] Error handling for invalid images

**Performance Testing**:
- [ ] Small images (< 1MB): Instant conversion
- [ ] Large images (10MB+): Completes in < 5 seconds
- [ ] No memory leaks after repeated conversions
- [ ] UI remains responsive during processing

**Accessibility Testing**:  
- [ ] Button is keyboard navigable (Tab/Space)
- [ ] Screen reader announces button state changes
- [ ] Tooltips are read by assistive technology
- [ ] High contrast themes display correctly

**Cross-Platform Testing**:
- [ ] Linux: Native GTK4 environment
- [ ] Windows: MSYS2 MinGW environment  
- [ ] macOS: Homebrew GTK4 environment
- [ ] Consistent behavior across all platforms

## Next Steps

After basic implementation:

1. **Unit Tests**: Create `tests/unit/test-image-processing.c` for conversion algorithms
2. **Integration Tests**: Add DoGTail tests for UI behavior  
3. **Documentation**: Update README.md with new feature description
4. **Localization**: Add translatable strings for internationalization
5. **Performance Optimization**: Consider async processing for very large images

## API Reference

**Key Functions**:
- `image_processor_convert_to_grayscale()`: Core conversion algorithm
- `hello_image_viewer_toggle_conversion()`: Public conversion interface
- `on_conversion_button_toggled()`: UI signal handler

**Properties**:
- `conversion-enabled`: Feature availability control
- `conversion-state`: Current image mode (color/grayscale)

**Signals**:
- `conversion-state-changed`: Emitted on mode change
- `conversion-error`: Emitted on processing failure

This quick start guide provides all essential information to implement the B&W conversion feature efficiently while maintaining code quality and user experience standards.