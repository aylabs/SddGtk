# Data Model: Image Black & White Conversion

**Feature**: Image B&W conversion button for HelloImageViewer  
**Created**: February 3, 2026  
**Source**: [spec.md](spec.md) functional requirements

## Core Entities

### ConversionButton

**Purpose**: Toggle button that switches images between color and grayscale states

**Attributes**:
- `is_toggled`: Boolean state indicating conversion mode (false = color, true = B&W)
- `label_text`: Dynamic button text ("Convert to B&W" | "Restore Color")  
- `tooltip_text`: Dynamic tooltip ("Convert image to black and white" | "Restore original colors")
- `icon_name`: Dynamic icon ("image-filter-symbolic" | "image-restore-symbolic")
- `sensitive`: Button availability (disabled when no image loaded)

**Relationships**:
- **contained by** HelloImageViewer window toolbar
- **triggers** ImageProcessor conversion operations
- **updates** ImageDisplayState when toggled

**State Transitions**:
- `created` → `enabled` (when image is loaded)
- `enabled` → `converting` → `converted` (toggle to B&W)
- `converted` → `restoring` → `enabled` (toggle back to color)
- `enabled|converted` → `disabled` (when no image loaded)

**Validation Rules**:
- Must be disabled when no image is present
- Must provide immediate visual feedback during state changes
- Must maintain consistent state with underlying image data
- Must be keyboard accessible and screen reader compatible

---

### ImageProcessor

**Purpose**: Utility class handling grayscale conversion algorithms and image data management

**Attributes**:
- `conversion_method`: Grayscale algorithm (luminance-based ITU-R BT.709)
- `processing_state`: Current operation state (idle, converting, restoring)
- `error_state`: Last operation error information
- `performance_metrics`: Conversion timing and memory usage data

**Operations**:
- `convert_to_grayscale(GdkPixbuf *original) → GdkPixbuf*`: Creates B&W version
- `validate_conversion(GdkPixbuf *result) → Boolean`: Quality verification
- `estimate_processing_time(image_size) → Duration`: Performance prediction

**Relationships**:
- **used by** HelloImageViewer for image processing
- **operates on** ImageData entities
- **reports to** ConversionButton for UI feedback

**Validation Rules**:
- Must preserve original image resolution and quality
- Must handle all supported image formats consistently
- Must provide error information for failed conversions
- Must operate within defined memory and performance constraints

---

### ImageData

**Purpose**: Enhanced image data management with B&W conversion state tracking

**Attributes**:
- `original_pixbuf`: GdkPixbuf* containing unmodified image data
- `current_pixbuf`: GdkPixbuf* for display (may be grayscale converted)
- `conversion_state`: Current image state (original, converted)
- `filename`: Source file path for reference
- `format_info`: Image format metadata (PNG, JPEG, etc.)
- `dimensions`: Width and height in pixels
- `memory_usage`: Current memory footprint tracking

**Relationships**:
- **managed by** HelloImageViewer instance
- **processed by** ImageProcessor during conversions
- **displayed via** GtkPicture widget

**State Transitions**:
- `loaded` → `original` (initial file load)
- `original` → `converting` → `converted` (B&W conversion)
- `converted` → `restoring` → `original` (color restoration)

**Validation Rules**:
- Original pixbuf must never be modified or released during session
- Current pixbuf must always point to valid display-ready data
- Memory usage must be tracked and bounded
- File format support must match existing HelloImageViewer capabilities

---

### HelloImageViewerExtension

**Purpose**: Enhanced HelloImageViewer class with B&W conversion functionality

**New Attributes**:
- `conversion_button`: GtkToggleButton for B&W conversion
- `image_data`: Enhanced ImageData with conversion support
- `conversion_enabled`: Feature availability flag
- `ui_template_path`: Path to extended UI template with button

**New Operations**:
- `toggle_conversion()`: Main conversion/restoration handler
- `update_button_state()`: Synchronize UI with image state
- `handle_conversion_error()`: Error state management
- `reset_to_original()`: Force restoration to color image

**Enhanced Relationships**:
- **extends** existing HelloImageViewer functionality
- **integrates** ConversionButton in window layout
- **manages** ImageProcessor lifecycle
- **maintains** ImageData consistency

**Validation Rules**:
- Must preserve all existing HelloImageViewer functionality
- Must handle multiple window instances independently
- Must gracefully degrade when conversion features unavailable
- Must maintain session-based state without persistence

---

## Resource Entities

### UI Template Extension (hello-image-viewer.ui)

**Purpose**: GtkBuilder XML template defining image viewer window with conversion button

**Enhanced Structure**:
```xml
<interface>
  <template class="HelloImageViewer" parent="GtkWindow">
    <property name="title">Image Viewer</property>
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
                <property name="tooltip-text">Convert to black and white</property>
                <property name="sensitive">false</property>
                <signal name="toggled" handler="on_conversion_button_toggled"/>
              </object>
            </child>
          </object>
        </child>
        <!-- Existing scrolled window with image -->
        <child>
          <object class="GtkScrolledWindow" id="scrolled_window">
            <property name="hexpand">true</property>
            <property name="vexpand">true</property>
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

**Validation Rules**:
- Must be valid GTK4 UI definition XML
- Must extend existing HelloImageViewer template structure  
- Must include proper accessibility attributes
- Must support internationalization with translatable properties

---

### Image Processing Library (image-processing.{c,h})

**Purpose**: Reusable C library for image format conversion operations

**Core Functions**:
```c
// Primary conversion function
GdkPixbuf* image_processor_convert_to_grayscale(GdkPixbuf *original, GError **error);

// Validation and utility functions  
gboolean image_processor_validate_pixbuf(GdkPixbuf *pixbuf);
gsize image_processor_estimate_memory_usage(gint width, gint height);
gdouble image_processor_calculate_conversion_progress(gsize processed, gsize total);
```

**Error Handling**:
- Comprehensive GError integration for conversion failures
- Memory allocation failure detection and cleanup
- Invalid image format handling
- Progress reporting for large image processing

**Validation Rules**:
- Must be thread-safe for potential future async processing
- Must handle all GdkPixbuf supported formats consistently
- Must provide detailed error information for debugging
- Must be unit testable independently of GTK UI

---

## Data Flow Diagrams

### Conversion Workflow

```text
User Clicks Button → ConversionButton.toggled
                          ↓
                   Check ImageData.conversion_state
                          ↓
    ┌─────────────────────┴─────────────────────┐
    ↓                                           ↓
[original] → ImageProcessor.convert_to_grayscale  [converted] → Use ImageData.original_pixbuf
    ↓                                           ↓
Update ImageData.current_pixbuf              Update ImageData.current_pixbuf
    ↓                                           ↓
    └─────────────── Update GtkPicture Display ─────────────────┘
                          ↓
              Update ConversionButton UI State
```

### Memory Management

```text
Image Load → Create ImageData.original_pixbuf (permanent)
                      ↓
             Set ImageData.current_pixbuf = original_pixbuf
                      ↓
   B&W Conversion → Create temporary grayscale pixbuf
                      ↓
             Update ImageData.current_pixbuf = converted_pixbuf
                      ↓
   Color Restore → Discard converted pixbuf
                      ↓
             Reset ImageData.current_pixbuf = original_pixbuf
```

## Integration Points

### With Existing HelloImageViewer

- **Minimal Changes**: Preserve existing API and functionality
- **Backward Compatibility**: All current features remain unchanged
- **Resource Integration**: Extend existing UI template and build system
- **Error Handling**: Integrate with existing image loading error patterns

### With GTK4 Framework

- **Widget Hierarchy**: Proper parent-child relationships in UI tree
- **Signal Management**: Standard GTK signal/slot connection patterns  
- **Memory Management**: GObject reference counting and disposal
- **Accessibility**: Full ARIA compliance with existing window structure

### With Build System

- **Meson Integration**: Add new source files to existing build configuration
- **Dependency Management**: Leverage existing GTK4 and GdkPixbuf dependencies
- **Testing Framework**: Extend existing Check-based unit test structure
- **Resource Compilation**: Include new UI templates in GResource bundles