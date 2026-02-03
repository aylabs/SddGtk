# Tasks: Image Blur Effect with Intensity Control

**Input**: Design documents from `/specs/003-image-blur-effect/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, contracts/ ✅

**Tests**: Test tasks included as per feature specification requirements

**Organization**: Tasks organized by feature phases enabling focused implementation and testing

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)  
- **[Story]**: Which user story this task belongs to (US1 = blur slider functionality)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and blur library foundation

- [x] T001 Create blur processor header file in src/lib/blur-processor.h
- [x] T002 Create blur cache header file in src/lib/blur-cache.h  
- [x] T003 [P] Add blur library dependencies to src/lib/meson.build
- [x] T004 [P] Setup blur test infrastructure in tests/unit/meson.build

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core blur processing infrastructure that MUST be complete before UI integration

**⚠️ CRITICAL**: No UI work can begin until this phase is complete

- [x] T005 Implement Gaussian kernel generation functions in src/lib/blur-processor.c
- [x] T006 [P] Implement separable convolution algorithm (horizontal pass) in src/lib/blur-processor.c
- [x] T007 [P] Implement separable convolution algorithm (vertical pass) in src/lib/blur-processor.c  
- [x] T008 [P] Implement LRU cache with memory management in src/lib/blur-cache.c
- [x] T009 Implement background thread processing framework in src/lib/blur-processor.c
- [x] T010 [P] Add blur parameter validation utilities in src/lib/blur-processor.c
- [x] T011 [P] Implement blur processor lifecycle management (create/destroy) in src/lib/blur-processor.c

**Checkpoint**: Foundation ready - blur library functional, UI integration can begin

---

## Phase 3: User Story 1 - Blur Slider Control (Priority: P1) 🎯 MVP

**Goal**: Users can apply real-time Gaussian blur effects using intensity slider (0.0-10.0)

**Independent Test**: Load image, move blur slider from 0 to 10, verify smooth blur application

### Tests for User Story 1

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [ ] T012 [P] [US1] Unit test for Gaussian blur algorithm in tests/unit/test-blur-processor.c
- [ ] T013 [P] [US1] Unit test for blur cache functionality in tests/unit/test-blur-cache.c
- [ ] T014 [P] [US1] Performance test for blur processing speed in tests/performance/blur-benchmarks.c
- [ ] T015 [P] [US1] Integration test for blur UI widgets in tests/integration/test-blur-ui.c

### Implementation for User Story 1

#### Blur Library Extension
- [ ] T016 [P] [US1] Implement blur_processor_apply_async() function in src/lib/blur-processor.c  
- [ ] T017 [P] [US1] Implement blur_processor_cancel() function in src/lib/blur-processor.c
- [ ] T018 [P] [US1] Implement blur_cache_get() and blur_cache_put() in src/lib/blur-cache.c
- [ ] T019 [P] [US1] Add progressive rendering support for real-time preview in src/lib/blur-processor.c

#### UI Integration
- [x] T020 [US1] Extend HelloImageViewer struct with blur support in src/hello-app/hello-image-viewer.h
- [x] T021 [US1] Add blur slider widgets to UI template in src/hello-app/resources/hello-image-viewer.ui
- [x] T022 [US1] Implement blur slider signal handlers in src/hello-app/hello-image-viewer.c
- [x] T023 [US1] Add blur intensity display and icon in src/hello-app/hello-image-viewer.c
- [x] T024 [US1] Implement blur processing callbacks and UI updates in src/hello-app/hello-image-viewer.c

#### Feature Integration  
- [x] T025 [US1] Integrate blur with existing B&W conversion in src/hello-app/hello-image-viewer.c
- [x] T026 [US1] Add image loading/unloading blur handlers in src/hello-app/hello-image-viewer.c
- [x] T027 [US1] Implement blur reset functionality in src/hello-app/hello-image-viewer.c
- [x] T028 [US1] Add blur slider styling in src/hello-app/resources/image-viewer.css

**Checkpoint**: User Story 1 complete - blur slider functional with real-time preview

---

## Phase 4: Polish & Cross-cutting Concerns  

**Purpose**: Performance optimization, error handling, and user experience enhancements

- [ ] T029 [P] Add comprehensive error handling for memory allocation failures
- [ ] T030 [P] Implement blur processing cancellation on rapid slider changes  
- [ ] T031 [P] Add blur performance monitoring and optimization
- [ ] T032 [P] Add accessibility support for blur slider (keyboard navigation, screen reader)
- [ ] T033 [P] Add blur effect visual feedback during processing (loading indicator)
- [ ] T034 [P] Optimize cache eviction strategy for memory pressure scenarios
- [ ] T035 [P] Add blur quality validation and edge case handling

---

## Dependencies

**User Story Completion Order**:
1. **Setup Phase** → **Foundational Phase** (Sequential - foundational depends on setup)
2. **Foundational Phase** → **User Story 1** (User story depends on complete foundation)  
3. **User Story 1** → **Polish Phase** (Polish enhances completed user story)

**Within Phases**:
- Setup: All tasks can run in parallel after T001 completes
- Foundational: T006-T007 parallel after T005, T008-T011 parallel independently
- User Story 1: Tests (T012-T015) parallel first, Library (T016-T019) parallel, UI (T020-T024) sequential, Integration (T025-T028) sequential after UI
- Polish: All tasks can run in parallel

**Critical Path**: T001 → T005 → T009 → T020 → T022 → T024 (Setup → Kernel → Threading → UI Structure → Signal Handling → Callbacks)

## Parallel Execution Examples

### Foundational Phase Parallelization  
```bash
# Team Member 1: Core blur algorithms
T005 (Gaussian kernels) → T006 (Horizontal pass) → T007 (Vertical pass)

# Team Member 2: Cache and memory management  
T008 (LRU cache) → T010 (Parameter validation) 

# Team Member 3: Threading infrastructure
T009 (Background processing) → T011 (Processor lifecycle)
```

### User Story 1 Parallelization
```bash
# Team Member 1: Library API implementation
T016 (apply_async) → T017 (cancel) → T019 (progressive rendering)

# Team Member 2: Cache implementation  
T018 (cache get/put)

# Team Member 3: UI foundation (after T016-T018 complete)
T020 (struct extension) → T021 (UI template) → T022 (signal handlers)

# Team Member 4: Tests (can start immediately)
T012 (blur algorithm tests) → T013 (cache tests) → T014 (performance tests)
```

## Implementation Strategy  

**MVP Approach**: Focus on User Story 1 for initial working feature
- Core blur processing (T005-T011)
- Basic UI integration (T020-T024)  
- Essential B&W compatibility (T025)

**Incremental Delivery**:
1. **Sprint 1**: Foundational Phase → Working blur library
2. **Sprint 2**: User Story 1 → Complete blur slider functionality
3. **Sprint 3**: Polish Phase → Production-ready feature

**Independent Testing**: Each phase includes validation tasks to ensure quality and catch issues early. User Story 1 can be fully tested and delivered independently.

**Performance Targets**:
- T014 validates <500ms processing for HD images  
- T019 enables <100ms progressive preview
- T031 optimizes for production performance requirements

**Total Tasks**: 35 tasks
- Setup: 4 tasks
- Foundational: 7 tasks  
- User Story 1: 17 tasks (5 tests + 12 implementation)
- Polish: 7 tasks

**Estimated Timeline**: 5-7 days with 2-3 developers working in parallel