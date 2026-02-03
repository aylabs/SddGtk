# Hello GTK Application

A minimal "Hello World" GTK4 application demonstrating cross-platform GUI development following the project's GTK-First constitutional requirements.

## Overview

This application serves as a foundation example for GTK4 development, showcasing:

- **GTK-First Architecture**: Exclusive use of GTK4 framework
- **Cross-Platform Compatibility**: Runs on Linux, Windows, and macOS
- **GTK Best Practices**: GtkBuilder UI templates, proper signal handling, GObject patterns
- **Library-First Design**: Reusable components and clean separation of concerns
- **Comprehensive Testing**: Unit tests, integration tests, and cross-platform validation

## Features

- Simple GTK4 application window with "Hello World!" message
- Explicit "Close" button for application termination
- "Open Image" button with file chooser dialog for image selection
- Dedicated image viewer window for displaying selected images
- **Black & White Image Conversion**: Toggle button to convert images between color and grayscale modes
  - Instant conversion using ITU-R BT.709 luminance algorithm
  - Original image restoration with perfect quality preservation
  - Independent state per viewer window
  - Processing feedback and error handling
  - Full accessibility support
- Support for common image formats (PNG, JPEG, GIF, SVG, WebP)
- Proper application lifecycle management
- Clean window close behavior
- Cross-platform resource handling
- Template-based UI definition with GtkBuilder

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install libgtk-4-dev build-essential meson pkg-config
```

### Windows (MSYS2)
```bash
# Install MSYS2 from https://www.msys2.org/
# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-gtk4
pacman -S mingw-w64-x86_64-meson  
pacman -S mingw-w64-x86_64-toolchain
```

### macOS (Homebrew)
```bash
# Install Homebrew from https://brew.sh/
brew install gtk4 meson pkg-config
```

### Optional Testing Dependencies
```bash
# For unit testing (Check framework)
# Linux: sudo apt install check libcheck-dev
# macOS: brew install check
# Windows: pacman -S mingw-w64-x86_64-check

# For integration testing (DoGTail)
pip3 install dogtail
```

## Building

### Using Meson (Recommended)

```bash
# Setup build directory
meson setup build

# Compile
cd build
meson compile

# Run application
./hello-app
```

### Using CMake (Alternative)

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make

# Run application
./hello-app
```

## Usage

### Basic Application Flow

1. Launch the application: `./hello-app`
2. Click "Open Image" to select an image file
3. Image opens in a dedicated viewer window
4. Use the B&W conversion toggle button to convert between color and grayscale

### Image B&W Conversion Feature

The image viewer includes a toggle button in the header bar for converting images:

- **Convert to B&W**: Click the filter icon to convert the image to grayscale using ITU-R BT.709 algorithm
- **Restore Color**: Click the restore icon to return to the original color image
- **Independent Windows**: Each viewer window maintains its own conversion state
- **Quality Preservation**: Original image data is preserved, ensuring perfect restoration
- **Performance**: HD images (1920x1080) convert in under 10ms
- **Accessibility**: Full screen reader support and keyboard navigation

### Supported Image Formats

- PNG (Portable Network Graphics)
- JPEG (Joint Photographic Experts Group)
- GIF (Graphics Interchange Format)  
- SVG (Scalable Vector Graphics)
- WebP (Web Picture format)

### Keyboard Shortcuts

- **Ctrl+O**: Open image file (from main window)
- **Ctrl+W**: Close current window
- **Space**: Toggle B&W conversion (when image viewer is focused)
- **Escape**: Close current window

## Testing

### Unit Tests

```bash
# From build directory (Meson)
meson test

# Or run individual tests
./test-hello-application
./test-hello-window
./test-image-processing
./test-image-viewer-bw
```

### Performance Tests

```bash
# Test B&W conversion performance
python3 ../tests/performance/test_performance.py
```

### Validation Tests

```bash
# Test cross-platform compatibility and accessibility
python3 ../tests/validation/test_validation.py
```

### Integration Tests

```bash
# Run integration tests (requires DoGTail)
python3 ../tests/integration/test-application.py
```

### Cross-Platform Tests

```bash
# Run platform-specific test scripts
../tests/platform/test-linux.sh      # Linux
../tests/platform/test-windows.sh    # Windows  
../tests/platform/test-macos.sh      # macOS
```

## Project Structure

```
src/
├── hello-app/
│   ├── main.c                    # Application entry point
│   ├── hello-application.{c,h}   # GtkApplication subclass
│   ├── hello-window.{c,h}        # GtkApplicationWindow subclass
│   ├── hello-image-viewer.{c,h}  # Image viewer window class with B&W conversion
│   ├── hello-app.gresource.xml   # Resource bundle definition
│   └── resources/
│       ├── hello-window.ui       # GtkBuilder UI template
│       └── hello-image-viewer.ui # Image viewer UI template with conversion button
├── lib/
│   ├── gtk-utils.{c,h}           # Reusable GTK utilities
│   └── image-processing.{c,h}    # Image conversion algorithms (ITU-R BT.709)
tests/
├── unit/                         # C unit tests using Check framework
│   ├── test-hello-application.c  # Application unit tests
│   ├── test-hello-window.c       # Window unit tests
│   ├── test-image-processing.c   # Image conversion unit tests
│   └── test-image-viewer-bw.c    # B&W feature unit tests
├── integration/                  # Python integration tests using DoGTail  
├── performance/                  # Performance and memory tests
│   └── test_performance.py       # Image conversion performance tests
├── validation/                   # Cross-platform validation tests
│   └── test_validation.py        # Platform compatibility validation
└── platform/                    # Cross-platform validation scripts
```

## Development

### Debug Build

```bash
# Meson debug build
meson setup build --buildtype=debug
cd build
meson compile

# Run with GTK Inspector
GTK_DEBUG=interactive ./hello-app
```

### Code Standards

- **C Standard**: C11
- **GTK Version**: GTK4 (4.0+)
- **Coding Style**: GTK/GNOME conventions
- **Memory Management**: GObject reference counting
- **UI Definition**: GtkBuilder templates
- **Testing**: TDD with unit and integration tests

### Constitutional Compliance

This application follows the project's GTK-First constitutional requirements:

✅ **I. GTK-First GUI Development**: Exclusive use of GTK4 framework  
✅ **II. Cross-Platform Compatibility**: Identical behavior across platforms  
✅ **III. GTK Best Practices**: GtkBuilder, proper signal handling, GObject patterns  
✅ **IV. Library-First Architecture**: Modular components with clear interfaces  
✅ **V. Testing Requirements**: Comprehensive unit, integration, and platform testing  

## Architecture

### Application Flow

1. **main()** initializes GTK and creates HelloApplication
2. **HelloApplication** manages application lifecycle and window creation
3. **HelloWindow** displays UI using GtkBuilder template with greeting and action buttons
4. **HelloImageViewer** provides dedicated windows for image display
5. **gtk-utils** provides reusable GTK functionality
6. **Resources** are embedded using GResource system

### GObject Hierarchy

- **HelloApplication** → GtkApplication → GApplication → GObject
- **HelloWindow** → GtkApplicationWindow → GtkWindow → GtkWidget → GObject
- **HelloImageViewer** → GtkWindow → GtkWidget → GObject

### Memory Management

- Automatic GObject reference counting
- Proper dispose/finalize implementation
- Template-based signal connection management
- Resource cleanup on application shutdown

## Troubleshooting

### Build Issues

**GTK4 not found**:
```bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

**Resource compilation fails**:
```bash
# Ensure glib-compile-resources is available
which glib-compile-resources
```

### Runtime Issues

**Application doesn't start**:
```bash
# Check dependencies
ldd ./hello-app  # Linux
otool -L ./hello-app  # macOS

# Check GTK debug output
GTK_DEBUG=all ./hello-app
```

**Window not displaying**:
```bash
# Check display environment (Linux)
echo $DISPLAY
export DISPLAY=:0
```

## Next Steps

This minimal application can be extended with:

- **Menu System**: GMenu-based application menus
- **Preferences**: Settings dialog with persistent configuration  
- **Internationalization**: gettext-based translations
- **About Dialog**: Standard GTK about dialog
- **Custom Widgets**: Reusable GUI components
- **CSS Theming**: Custom application styling

## Contributing

Follow the established development workflow:

1. Write tests first (TDD approach)
2. Implement functionality
3. Verify cross-platform compatibility
4. Update documentation
5. Follow GTK/GNOME coding standards

## License

MIT License - See LICENSE file for details.

## Documentation

### Hello Application (Base)
- **Specification**: [specs/001-hello-app/spec.md](specs/001-hello-app/spec.md)
- **Implementation Plan**: [specs/001-hello-app/plan.md](specs/001-hello-app/plan.md)
- **Tasks**: [specs/001-hello-app/tasks.md](specs/001-hello-app/tasks.md)
- **API Contracts**: [specs/001-hello-app/contracts/](specs/001-hello-app/contracts/)
- **Quick Start**: [specs/001-hello-app/quickstart.md](specs/001-hello-app/quickstart.md)

### Image B&W Conversion Feature
- **Feature Specification**: [specs/002-image-bw-convert/spec.md](specs/002-image-bw-convert/spec.md)
- **Implementation Plan**: [specs/002-image-bw-convert/plan.md](specs/002-image-bw-convert/plan.md)
- **Task Breakdown**: [specs/002-image-bw-convert/tasks.md](specs/002-image-bw-convert/tasks.md)
- **Technical Research**: [specs/002-image-bw-convert/research.md](specs/002-image-bw-convert/research.md)
- **Data Model**: [specs/002-image-bw-convert/data-model.md](specs/002-image-bw-convert/data-model.md)
- **API Contracts**: [specs/002-image-bw-convert/contracts/](specs/002-image-bw-convert/contracts/)
- **Quick Start Guide**: [specs/002-image-bw-convert/quickstart.md](specs/002-image-bw-convert/quickstart.md)