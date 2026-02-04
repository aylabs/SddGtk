# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Image Blur Effect Feature** - Real-time Gaussian blur with intensity control
  - Horizontal blur intensity slider (range 0.0 to 10.0)
  - Real-time preview with smooth interaction
  - High-performance Gaussian blur processing engine
  - Intelligent LRU cache system for blur results
  - Memory-efficient cache with automatic cleanup
  - Independent blur state per image viewer window
  - Keyboard shortcuts for blur control (Ctrl+0, Ctrl+Plus, Ctrl+Minus)
  - HD image support (1920x1080) with sub-50ms processing

### Technical Infrastructure
- **Blur Processing Engine** (`blur-processor.c/h`)
  - Gaussian blur algorithm implementation with configurable sigma
  - High-performance pixel processing with proper edge handling
  - Memory-safe operations with bounds checking
  - Error handling and input validation

- **Blur Cache System** (`blur-cache.c/h`)
  - LRU (Least Recently Used) cache implementation
  - Configurable cache size limits (default 100MB)
  - Automatic memory management and cleanup
  - Cache key generation for blur parameters
  - Performance statistics and monitoring

- **Comprehensive Testing Suite**
  - Unit tests for blur processor (5 test cases)
  - Unit tests for blur cache system (8 test cases)
  - Integration tests for complete blur workflow (6 test cases)
  - Performance tests and memory profiling
  - Cross-platform compatibility testing
  - CI/CD integration with headless environment support

### Improved
- **Test Infrastructure Robustness**
  - GTK initialization fixes for headless environments
  - Graceful test skipping when display unavailable
  - CI/CD compatibility improvements
  - All tests now handle headless execution properly

### Fixed
- Test timeout issues in CI/CD environments
- GTK initialization failures in headless systems
- Test isolation issues in blur cache tests
- Memory management improvements

## [v1.1.0] - Previous Version

### Added
- **Image Black & White Conversion Feature**
  - Toggle button for color/grayscale conversion
  - ITU-R BT.709 luminance algorithm implementation
  - Perfect quality preservation and restoration
  - Independent state per image viewer window
  - Full accessibility support

### Added
- Image viewer window with file chooser dialog
- Support for common image formats (PNG, JPEG, GIF, SVG, WebP)
- Proper application lifecycle management
- GtkBuilder-based UI templates
- Cross-platform compatibility (Linux, macOS, Windows)

## [v1.0.0] - Initial Release

### Added
- Basic GTK4 "Hello World" application
- Simple application window with greeting message
- Close button functionality
- GTK-First architecture foundation
- Comprehensive testing framework
- Cross-platform build system (Meson/CMake)
- GObject-based architecture
- Resource embedding with GResource

### Documentation
- Complete README with build instructions
- Cross-platform setup guides
- Testing documentation
- Architecture overview
- Constitutional compliance verification

---

## Versioning Strategy

This project uses semantic versioning:
- **MAJOR**: Incompatible API changes
- **MINOR**: New functionality in backward-compatible manner
- **PATCH**: Backward-compatible bug fixes

## Development Workflow

1. Features are developed in dedicated branches
2. Comprehensive testing required before merge
3. Documentation updated with each release
4. Cross-platform compatibility verified
5. Performance benchmarks maintained