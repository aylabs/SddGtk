# Implementation Plan: Image Black & White Conversion

**Branch**: `002-image-bw-convert` | **Date**: February 3, 2026 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/002-image-bw-convert/spec.md`

## Summary

Add a toggle button to the existing HelloImageViewer window that converts displayed images between color and grayscale (black and white) states. The feature preserves original image data for instant restoration, maintains independent state per viewer window, and provides clear visual feedback through button state changes.

## Technical Context

**Language/Version**: C11 with GTK4 framework (version 4.0+)  
**Primary Dependencies**: GTK4, GtkPicture, GdkPixbuf, GObject system for image processing  
**Storage**: In-memory retention of original image data for color restoration (session-based)  
**Testing**: Check framework for C unit tests, DoGTail for integration testing  
**Target Platform**: Cross-platform (Linux, Windows, macOS via GTK4)  
**Project Type**: Single GTK4 application extending existing HelloImageViewer class  
**Performance Goals**: Image conversion in under 1 second, support images up to 50MB  
**Constraints**: Preserve 100% image quality and resolution during conversion  
**Scale/Scope**: Individual image viewer windows, session-based state management  

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**✅ I. GTK-First GUI Development**: Feature exclusively uses GTK4 framework and GdkPixbuf for image processing
**✅ II. Cross-Platform Compatibility**: Conversion functionality will work identically across Linux, Windows, macOS  
**✅ III. GTK Best Practices**: Extends existing GObject HelloImageViewer class with proper signal handling and GtkBuilder UI templates
**✅ IV. Library-First Architecture**: Modular conversion logic in reusable image-processing library with documented APIs
**✅ V. Testing Requirements**: Unit tests for conversion logic, integration tests for UI behavior, cross-platform validation

**POST-DESIGN RE-EVALUATION**: ✅ **PASSED** 
- All design artifacts follow GTK-First constitutional requirements
- UI templates use GtkBuilder with proper GTK4 widgets (GtkToggleButton, GtkHeaderBar)
- Image processing leverages native GdkPixbuf APIs exclusively  
- Memory management follows GObject reference counting patterns
- Reusable image-processing library provides clean API separation
- Cross-platform compatibility maintained through GTK4 abstractions
- Comprehensive testing strategy covers all constitutional requirements

**GATE EVALUATION**: ✅ **PASSED** - All constitutional requirements satisfied pre and post-design

## Project Structure

### Documentation (this feature)

```text
specs/002-image-bw-convert/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
src/
├── hello-app/
│   ├── hello-image-viewer.{c,h}  # MODIFY: Add B&W conversion functionality
│   └── resources/
│       └── hello-image-viewer.ui # CREATE: UI template with conversion button
└── lib/
    └── image-processing.{c,h}    # CREATE: Reusable image conversion utilities

tests/
├── unit/
│   ├── test-image-processing.c   # CREATE: Test conversion algorithms
│   └── test-image-viewer-bw.c    # CREATE: Test HelloImageViewer B&W features
└── integration/
    └── test-bw-conversion.py     # CREATE: End-to-end UI testing
```
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this feature. Delete unused options and expand the chosen structure with
  real paths (e.g., apps/admin, packages/something). The delivered plan must
  not include Option labels.
-->

```text
# [REMOVE IF UNUSED] Option 1: Single project (DEFAULT)
src/
├── models/
├── services/
├── cli/
└── lib/

tests/
├── contract/
├── integration/
└── unit/

# [REMOVE IF UNUSED] Option 2: Web application (when "frontend" + "backend" detected)
backend/
├── src/
│   ├── models/
│   ├── services/
│   └── api/
└── tests/

frontend/
├── src/
│   ├── components/
│   ├── pages/
│   └── services/
└── tests/

# [REMOVE IF UNUSED] Option 3: Mobile + API (when "iOS/Android" detected)
api/
└── [same as backend above]

ios/ or android/
└── [platform-specific structure: feature modules, UI flows, platform tests]
```

**Structure Decision**: [Document the selected structure and reference the real
directories captured above]

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
