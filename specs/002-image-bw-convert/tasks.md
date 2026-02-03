# Tasks: Image Black & White Conversion

**Input**: Design documents from `/specs/002-image-bw-convert/`
**Prerequisites**: plan.md (technical architecture), spec.md (user stories P1-P3), research.md (GTK4/GdkPixbuf decisions), data-model.md (entities), contracts/ (API interfaces)

**Branch**: `002-image-bw-convert`
**Feature**: Add B&W conversion toggle button to HelloImageViewer windows

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)  
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and core image processing foundation

- [ ] T001 Create image processing library structure in src/lib/image-processing.{c,h}
- [ ] T002 [P] Update meson.build to include image-processing library compilation
- [ ] T003 [P] Update CMakeLists.txt to include image-processing library (alternative build)
- [ ] T004 [P] Create unit test structure in tests/unit/test-image-processing.c

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core image conversion functionality that MUST be complete before ANY user story UI can work

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T005 Implement `image_processor_convert_to_grayscale()` in src/lib/image-processing.c using ITU-R BT.709 algorithm
- [ ] T006 [P] Implement `image_processor_validate_pixbuf()` validation function in src/lib/image-processing.c
- [ ] T007 [P] Implement `image_processor_estimate_memory_usage()` utility in src/lib/image-processing.c
- [ ] T008 [P] Define `ImageProcessorError` error domain and handling in src/lib/image-processing.h
- [ ] T009 [P] Create comprehensive unit tests for conversion algorithms in tests/unit/test-image-processing.c
- [ ] T010 Validate conversion quality and performance with test images (PNG, JPEG, GIF formats)

**Checkpoint**: Image processing foundation ready - UI integration can now begin in parallel

---

## Phase 3: User Story 1 - Basic Image Conversion (Priority: P1) 🎯 MVP

**Goal**: Toggle button converts images between color and B&W, with instant restoration

**Independent Test**: Open image viewer, click B&W button to convert, click again to restore color

### Implementation for User Story 1

- [ ] T011 [P] [US1] Create hello-image-viewer.ui template in src/hello-app/resources/hello-image-viewer.ui
- [ ] T012 [US1] Add GtkHeaderBar with GtkToggleButton to hello-image-viewer.ui template
- [ ] T013 [US1] Update hello-image-viewer.h to include conversion button widget and state properties
- [ ] T014 [US1] Extend hello_image_viewer_init() to load UI template from GResource in src/hello-app/hello-image-viewer.c
- [ ] T015 [US1] Add private members (original_pixbuf, converted_pixbuf, is_converted) to HelloImageViewer struct
- [ ] T016 [US1] Implement `on_conversion_button_toggled()` signal handler in src/hello-app/hello-image-viewer.c
- [ ] T017 [US1] Update `hello_image_viewer_load_image()` to enable conversion button and store original pixbuf
- [ ] T018 [US1] Implement proper pixbuf memory management in dispose/finalize methods

**Checkpoint**: Basic B&W conversion fully functional - user can toggle any loaded image

---

## Phase 4: User Story 2 - Visual Feedback and Button States (Priority: P2)

**Goal**: Clear visual indication of conversion state with dynamic button appearance

**Independent Test**: Observe button icons, tooltips, and states during conversion operations

### Implementation for User Story 2

- [ ] T019 [P] [US2] Add dynamic icon switching logic (image-filter-symbolic ↔ image-restore-symbolic) in src/hello-app/hello-image-viewer.c
- [ ] T020 [P] [US2] Add dynamic tooltip updates ("Convert to B&W" ↔ "Restore Color") in src/hello-app/hello-image-viewer.c
- [ ] T021 [P] [US2] Implement button sensitivity management (disabled when no image) in src/hello-app/hello-image-viewer.c
- [ ] T022 [US2] Add processing state feedback with temporary button disable during conversion
- [ ] T023 [US2] Implement error state handling with user-friendly error dialogs
- [ ] T024 [US2] Add accessibility attributes and ARIA labels for screen readers

**Checkpoint**: Button provides comprehensive visual and accessibility feedback

---

## Phase 5: User Story 3 - Multiple Window Support (Priority: P3)

**Goal**: Each image viewer window maintains independent conversion state

**Independent Test**: Open multiple images, convert some to B&W, verify independent state per window

### Implementation for User Story 3

- [ ] T025 [P] [US3] Verify HelloImageViewer instance isolation in window management
- [ ] T026 [P] [US3] Test per-window state persistence during session in tests/unit/test-image-viewer-bw.c
- [ ] T027 [US3] Implement session-based state tracking for multiple viewer instances
- [ ] T028 [US3] Add memory management for multiple concurrent conversions
- [ ] T029 [US3] Validate independent window behavior with integration testing

**Checkpoint**: Multi-window conversion workflows fully supported

---

## Phase 6: Testing & Quality Assurance

**Purpose**: Comprehensive validation of all functionality

- [ ] T030 [P] Create DoGTail integration test for basic conversion workflow in tests/integration/test-bw-conversion.py
- [ ] T031 [P] Create unit tests for HelloImageViewer B&W extension in tests/unit/test-image-viewer-bw.c  
- [ ] T032 [P] Add performance tests for large images (10MB+, 4K resolution) in tests/unit/test-image-processing.c
- [ ] T033 [P] Add memory leak detection tests with Valgrind integration
- [ ] T034 Add cross-platform validation tests (Linux, Windows, macOS)
- [ ] T035 [P] Create accessibility compliance tests for button and keyboard navigation
- [ ] T036 Add edge case testing (grayscale images, corrupted files, memory constraints)

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Production readiness and documentation

- [ ] T037 [P] Update meson.build resource bundle to include hello-image-viewer.ui
- [ ] T038 [P] Update CMakeLists.txt resource compilation for hello-image-viewer.ui
- [ ] T039 [P] Add internationalization support for button tooltips and error messages
- [ ] T040 [P] Update README.md to document B&W conversion feature usage
- [ ] T041 [P] Add feature documentation to quickstart guide
- [ ] T042 Create CSS styling for conversion button states and theming support
- [ ] T043 [P] Add debug logging for troubleshooting conversion operations
- [ ] T044 Performance optimization for conversion caching and memory usage

---

## Dependencies

**User Story Completion Order**:
1. **User Story 1 (P1)**: Core conversion functionality - Required for MVP
2. **User Story 2 (P2)**: Visual feedback - Enhances User Story 1, can be developed in parallel after T018
3. **User Story 3 (P3)**: Multi-window support - Independent of other stories, can be developed in parallel

**Parallel Execution Opportunities**:
- **After T010 (Foundation complete)**: All User Story tasks can begin in parallel
- **UI Template work (T011-T012)**: Can start immediately after foundation
- **Visual feedback (T019-T024)**: Can start after T016 (signal handler) is complete
- **Testing tasks (T030-T036)**: Can be developed alongside implementation
- **Polish tasks (T037-T044)**: Can start after User Story 1 is functional

## Implementation Strategy

**MVP Delivery**: Complete Phase 1, 2, and Phase 3 (User Story 1) for a fully functional basic B&W conversion feature.

**Incremental Enhancement**: Add Phase 4 (User Story 2) for improved UX, then Phase 5 (User Story 3) for multi-window workflows.

**Quality Gate**: Phase 6 testing must pass before production deployment.

**Total Tasks**: 44 tasks organized across 7 phases with clear dependencies and parallel execution opportunities.

## Validation Criteria

**Phase 3 Complete (MVP)**:
- [ ] Any image can be converted to B&W and restored to color
- [ ] Button toggles correctly between states
- [ ] Memory management prevents leaks
- [ ] Basic error handling works

**Phase 4 Complete (Enhanced UX)**:
- [ ] Button provides clear visual feedback
- [ ] Icons and tooltips change appropriately
- [ ] Accessibility features work with screen readers
- [ ] Error states display user-friendly messages

**Phase 5 Complete (Multi-window)**:
- [ ] Multiple image viewers work independently
- [ ] Each window maintains its own conversion state
- [ ] No cross-window interference or memory issues

**All Phases Complete**:
- [ ] Performance meets requirements (< 1 second for typical images)
- [ ] Cross-platform compatibility validated
- [ ] Memory usage stays within bounds (< 3x image size)
- [ ] All tests pass including edge cases