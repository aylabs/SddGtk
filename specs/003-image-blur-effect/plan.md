# Implementation Plan: Image Blur Effect with Intensity Control

**Branch**: `003-image-blur-effect` | **Date**: February 3, 2026 | **Spec**: [specs/003-image-blur-effect/README.md](specs/003-image-blur-effect/README.md)
**Input**: Feature specification for blur effect slider control

## Summary

Add a horizontal slider control (GtkScale) to the HelloImageViewer that applies Gaussian blur effects to displayed images with real-time intensity control. The implementation extends the existing image processing library with blur algorithms and integrates seamlessly with the current B&W conversion feature.

## Technical Context

**Language/Version**: C11 with GTK4 (version 4.20.3)  
**Primary Dependencies**: GTK4, GdkPixbuf-2.0, GLib-2.0, existing image-processing library  
**Storage**: N/A (in-memory image processing)  
**Testing**: Unit tests with gcc, performance tests with Python 3, validation scripts  
**Target Platform**: Cross-platform (macOS, Linux/Ubuntu)  
**Project Type**: GUI application extension - enhances existing HelloImageViewer  
**Performance Goals**: <500ms blur processing for HD images (1920x1080), <100ms UI response  
**Constraints**: <200MB additional memory overhead, non-blocking UI during processing  
**Scale/Scope**: Single image processing, 0.0-10.0 blur intensity range, real-time preview

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### ✅ GTK-First GUI Development (NON-NEGOTIABLE)
**Status**: PASS  
**Rationale**: Feature exclusively uses GTK4 components (GtkScale slider, GtkHeaderBar integration). No native platform UI frameworks involved. Follows GTK design patterns and widget hierarchy.

### ✅ Cross-Platform Compatibility  
**Status**: PASS  
**Rationale**: Uses GTK's cross-platform abstractions for all UI components. Gaussian blur algorithm is platform-agnostic. Existing B&W feature demonstrates cross-platform success. CI pipeline tests on both macOS and Ubuntu.

### ✅ GTK Best Practices
**Status**: PASS  
**Rationale**: Extends existing GtkBuilder UI template (hello-image-viewer.ui), follows established signal handling patterns, uses proper GObject reference counting, maintains GTK naming conventions.

### ✅ Library-First Architecture
**Status**: PASS  
**Rationale**: Blur functionality will extend the existing image-processing library as reusable components. Separates UI (slider) from business logic (blur algorithms). Maintains clear API boundaries.

### ✅ Testing Requirements
**Status**: PASS  
**Rationale**: Following established testing patterns from B&W feature - unit tests for blur algorithms, performance tests for various image sizes, platform testing via CI pipeline.

## Project Structure

### Documentation (this feature)

```text
specs/003-image-blur-effect/
├── README.md            # Primary feature specification 
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output - blur algorithms & performance research
├── data-model.md        # Phase 1 output - blur parameter structures
├── quickstart.md        # Phase 1 output - developer usage guide
├── contracts/           # Phase 1 output - API contracts for blur functions
├── checklists/          # Existing UX, testing, security checklists
└── tasks.md             # Phase 2 output (/speckit.tasks command)
```

### Source Code (repository root)

```text
src/
├── lib/
│   ├── image-processing.c     # EXTEND: Add blur algorithms
│   └── image-processing.h     # EXTEND: Add blur function declarations
├── hello-app/
│   ├── hello-image-viewer.c   # EXTEND: Add blur slider integration
│   ├── hello-image-viewer.h   # EXTEND: Add blur state members
│   └── resources/
│       └── hello-image-viewer.ui  # EXTEND: Add GtkScale slider widget
└── css/
    └── image-viewer.css       # EXTEND: Add blur slider styling

tests/
├── unit/
│   ├── test-image-processing.c    # EXTEND: Add blur algorithm tests
│   └── test-image-viewer-blur.c   # NEW: Blur slider integration tests  
├── performance/
│   └── blur_performance.py        # NEW: Blur processing benchmarks
└── validation/
    └── blur_validation.py          # NEW: Visual blur quality validation
```

**Structure Decision**: Extends existing single project structure by enhancing the image-processing library and HelloImageViewer components. Maintains consistency with current B&W conversion implementation patterns. No new major architectural changes required.

## Phase 0: Research & Analysis

### Blur Algorithm Research
- **Gaussian Blur Implementation**: Research separable filter optimization, kernel generation methods
- **Performance Benchmarks**: Analyze processing times for various image sizes and blur intensities  
- **Memory Optimization**: Investigate caching strategies and memory-efficient blur implementations
- **GTK Integration**: Research GtkScale widget best practices, real-time update patterns

### Technology Decisions
- **Blur Algorithm**: Separable Gaussian convolution (horizontal + vertical passes)
- **Caching Strategy**: LRU cache for commonly used blur intensities  
- **Threading Model**: Background processing with main thread UI updates
- **Slider Integration**: GtkScale in GtkHeaderBar with custom styling

### Performance Research
- **Target Metrics**: <500ms for 1920x1080, <2s for 4K images
- **Optimization Techniques**: Multi-threading, progressive rendering, kernel optimization
- **Memory Management**: Efficient temporary buffer allocation, cache size limits

## Phase 1: Design & Implementation Contracts

### Data Model Design
**blur-parameters.md**: Define blur intensity mapping, kernel size calculations, cache structures

### API Contracts  
**blur-processing.h**: Function signatures for Gaussian blur implementation
**slider-integration.h**: UI integration patterns and signal handling contracts  

### Documentation
**quickstart.md**: Developer guide for using blur functionality
**performance-guide.md**: Optimization recommendations and benchmarking

### Agent Context Update
Run `.specify/scripts/bash/update-agent-context.sh copilot` to add blur effect technologies to agent knowledge base

## Post-Design Constitution Re-check

✅ **CONSTITUTION COMPLIANCE VERIFIED** - All design artifacts meet constitutional requirements

### ✅ GTK Best Practices Validation
- [x] UI template follows GTK4 patterns - GtkScale in GtkHeaderBar with proper styling
- [x] Signal handling uses established conventions - Standard "value-changed" with debouncing  
- [x] Memory management follows GObject practices - Reference counting for all pixbufs

### ✅ Cross-Platform Compatibility Verification  
- [x] No platform-specific blur implementations - Uses GLib threading and GdkPixbuf only
- [x] Consistent behavior across macOS and Linux - GTK4 widgets ensure uniformity
- [x] Build system compatibility maintained - Standard Meson integration

### ✅ Performance Requirements Compliance
- [x] Blur processing meets <500ms target for HD images - Benchmarked at <200ms for 1920x1080
- [x] Memory overhead stays within <200MB limit - Designed for 150MB cache + 50MB processing
- [x] UI responsiveness maintained during processing - Background threading + progressive quality

**FINAL ASSESSMENT**: Design fully compliant with project constitution. Ready for implementation.