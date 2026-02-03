# Quick Start Guide: Hello GTK Application

**Version**: 1.0.0  
**Updated**: 2026-02-01  
**Prerequisites**: GTK4 development environment

## Overview

This guide provides step-by-step instructions for building, running, and testing the minimal GTK Hello World application across Linux, Windows, and macOS platforms.

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
# Install GTK4 development packages
sudo apt update
sudo apt install libgtk-4-dev build-essential meson pkg-config

# Verify installation
pkg-config --modversion gtk4
```

### Windows (MSYS2)
```bash
# Install MSYS2 from https://www.msys2.org/
# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-gtk4
pacman -S mingw-w64-x86_64-meson
pacman -S mingw-w64-x86_64-toolchain

# Verify installation
pkg-config --modversion gtk4
```

### macOS (Homebrew)
```bash
# Install Homebrew from https://brew.sh/
# Install GTK4 and development tools
brew install gtk4 meson pkg-config

# Verify installation
pkg-config --modversion gtk4
```

## Quick Build and Run

### Method 1: Using Meson (Recommended)

```bash
# Clone and navigate to project
cd gtk  # Your project root

# Create build directory
meson setup builddir
cd builddir

# Compile application
meson compile

# Run application
./hello-app
```

### Method 2: Using CMake (Alternative)

```bash
# Create build directory
mkdir build && cd build

# Configure build
cmake ..

# Compile application
make

# Run application
./hello-app
```

## Development Workflow

### 1. Project Structure Setup

```bash
# Create project structure
mkdir -p src/hello-app/resources
mkdir -p tests/{unit,integration,platform}
mkdir -p docs

# Create main source files
touch src/hello-app/main.c
touch src/hello-app/hello-application.{c,h}
touch src/hello-app/hello-window.{c,h}
touch src/hello-app/resources/hello-window.ui
```

### 2. Build Configuration

**meson.build** (place in project root):
```meson
project('hello-app', 'c',
  version: '1.0.0',
  default_options: [
    'warning_level=3',
    'c_std=c11',
  ]
)

gtk_dep = dependency('gtk4')

# Compile resources
gnome = import('gnome')
resources = gnome.compile_resources(
  'hello-resources',
  'src/hello-app/hello-app.gresource.xml',
  source_dir: 'src/hello-app'
)

executable('hello-app',
  [
    'src/hello-app/main.c',
    'src/hello-app/hello-application.c',
    'src/hello-app/hello-window.c',
    resources,
  ],
  dependencies: gtk_dep,
  install: true
)
```

### 3. Resource Setup

**hello-app.gresource.xml**:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<gresources>
  <gresource prefix="/com/example/HelloApp">
    <file preprocess="xml-stripblanks">resources/hello-window.ui</file>
  </gresource>
</gresources>
```

## Testing

### Unit Tests

```bash
# Install testing dependencies (Check framework)
# Linux:
sudo apt install check libcheck-dev

# Run unit tests
cd builddir
meson test
```

### Integration Tests

```bash
# Install DoGTail for GUI testing
pip3 install dogtail

# Run integration tests
python3 tests/integration/test-application.py
```

### Cross-Platform Testing

```bash
# Run platform-specific tests
tests/platform/test-linux.sh      # Linux
tests/platform/test-windows.sh    # Windows
tests/platform/test-macos.sh      # macOS
```

## Common Tasks

### Debug Build

```bash
# Meson debug build
meson setup builddir --buildtype=debug
cd builddir
meson compile

# Run with debugging
GTK_DEBUG=interactive ./hello-app
```

### Release Build

```bash
# Meson release build
meson setup builddir --buildtype=release
cd builddir
meson compile

# Create distribution package
meson dist
```

### Clean Build

```bash
# Clean build directory
cd builddir
meson compile --clean

# Or remove and recreate
cd ..
rm -rf builddir
meson setup builddir
```

## Running the Application

### Basic Launch

```bash
# From build directory
./hello-app

# Or with full path
/path/to/builddir/hello-app
```

### With Environment Variables

```bash
# Enable debug output
GTK_DEBUG=all ./hello-app

# Set custom theme
GTK_THEME=Adwaita:dark ./hello-app

# Enable accessibility debugging
GTK_DEBUG=a11y ./hello-app
```

### Command Line Options

```bash
# Display help
./hello-app --help

# Display version
./hello-app --version

# Run in different display
DISPLAY=:1 ./hello-app  # Linux
```

## Development Tips

### Rapid Development Cycle

```bash
# Automatic rebuild and test cycle
while inotifywait -e modify src/; do
    meson compile -C builddir && ./builddir/hello-app
done
```

### GTK Inspector

```bash
# Enable GTK Inspector for debugging
GTK_DEBUG=interactive ./hello-app

# Or set environment permanently
export GTK_DEBUG=interactive
./hello-app
```

### Memory Debugging

```bash
# Linux - Valgrind memory checking
valgrind --leak-check=full ./hello-app

# Sanitizer options during build
meson setup builddir -Db_sanitize=address,undefined
```

## Platform-Specific Notes

### Linux Specific

- **System Integration**: Desktop files in `/usr/share/applications/`
- **Icon Theme**: Icons in `/usr/share/icons/hicolor/`
- **Packaging**: Create `.deb` or `.rpm` packages for distribution

```bash
# Install system-wide (Linux)
sudo meson install -C builddir
```

### Windows Specific

- **DLL Dependencies**: Ensure GTK4 DLLs are in PATH or app directory
- **Installer**: Use NSIS or WiX for Windows installers
- **Runtime**: Bundle GTK4 runtime or use GTK4 installer

```bash
# Windows - Copy required DLLs
ldd hello-app.exe  # Check dependencies
cp /mingw64/bin/*.dll .  # Copy required libraries
```

### macOS Specific

- **Bundle Creation**: Create `.app` bundle for distribution
- **Framework Dependencies**: Embed GTK4 frameworks
- **Code Signing**: Required for distribution outside App Store

```bash
# macOS - Create application bundle
mkdir -p HelloApp.app/Contents/{MacOS,Resources}
cp hello-app HelloApp.app/Contents/MacOS/
cp Info.plist HelloApp.app/Contents/
```

## Troubleshooting

### Common Build Issues

**Issue**: GTK4 not found
```bash
# Solution: Set PKG_CONFIG_PATH
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

**Issue**: Resource files not found
```bash
# Solution: Check resource compilation
meson introspect builddir --targets | grep resources
```

**Issue**: Application doesn't start
```bash
# Solution: Check dependencies
ldd ./hello-app  # Linux
otool -L ./hello-app  # macOS
```

### Runtime Issues

**Issue**: Window not displaying
```bash
# Check DISPLAY environment (Linux)
echo $DISPLAY
export DISPLAY=:0
```

**Issue**: Theme issues
```bash
# Reset GTK settings
gsettings reset-recursively org.gnome.desktop.interface
```

### Performance Issues

**Issue**: Slow startup
```bash
# Profile startup time
time ./hello-app &
sleep 1
pkill hello-app
```

## Next Steps

### Extend the Application

1. **Add Menu Bar**: Implement application menu using GMenu
2. **Add Preferences**: Create preferences dialog with GtkDialog
3. **Add Internationalization**: Implement gettext for translations
4. **Add About Dialog**: Standard GTK About dialog implementation

### Advanced Features

1. **Custom Widgets**: Create reusable GTK widget components
2. **CSS Theming**: Custom application styling with GTK CSS
3. **Keyboard Shortcuts**: Implement application-wide shortcuts
4. **Drag and Drop**: File drag-and-drop functionality

### Distribution

1. **Package Creation**: Create installation packages for each platform
2. **Continuous Integration**: Set up automated build and testing
3. **Documentation**: Generate API documentation with GTK-Doc
4. **User Documentation**: Create user manual and help system

## Additional Resources

- **GTK4 Documentation**: https://docs.gtk.org/gtk4/
- **GTK4 Tutorial**: https://gtk-rs.org/gtk4-rs/stable/latest/book/
- **Meson Documentation**: https://mesonbuild.com/
- **GObject Tutorial**: https://developer.gnome.org/gobject/stable/

This quickstart guide provides the foundation for GTK4 development following the project's constitutional requirements and best practices. Follow these steps to get up and running quickly with GTK cross-platform development.