# Tasks: Minimal Hello Application

**Input**: Design documents from `/specs/001-hello-app/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Test tasks included based on constitutional testing requirements.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `tests/` at repository root (per plan.md structure)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic GTK development structure

- [X] T001 Create project directory structure: src/hello-app/, src/lib/, tests/unit/, tests/integration/, tests/platform/
- [X] T002 Initialize Meson build system with meson.build in project root
- [X] T003 [P] Create CMakeLists.txt alternative build configuration
- [X] T004 [P] Configure GTK4 resource compilation with hello-app.gresource.xml
- [X] T005 [P] Setup development tooling configuration (.gitignore, .editorconfig)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core GTK infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T006 Create HelloApplication GObject class foundation in src/hello-app/hello-application.h
- [X] T007 Implement HelloApplication basic structure in src/hello-app/hello-application.c
- [X] T008 [P] Create HelloWindow GObject class foundation in src/hello-app/hello-window.h
- [X] T009 [P] Implement HelloWindow basic structure in src/hello-app/hello-window.c
- [X] T010 [P] Create GTK utility functions in src/lib/gtk-utils.c
- [X] T011 [P] Create main application entry point in src/hello-app/main.c
- [X] T012 Setup testing infrastructure with Check framework in tests/unit/
- [X] T013 [P] Configure DoGTail integration testing setup in tests/integration/

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Launch and View Hello Message (Priority: P1) 🎯 MVP

**Goal**: Create GTK window that displays "Hello World!" message with proper application lifecycle

**Independent Test**: Launch application and verify window appears with greeting message, closes cleanly

### Tests for User Story 1

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [X] T014 [P] [US1] Unit test for HelloApplication creation in tests/unit/test-hello-application.c
- [X] T015 [P] [US1] Unit test for HelloWindow creation in tests/unit/test-hello-window.c  
- [X] T016 [P] [US1] Integration test for application launch in tests/integration/test-application.py
- [X] T017 [P] [US1] Integration test for window display in tests/integration/test-window-display.py
- [X] T018 [P] [US1] Unit test for close button functionality in tests/unit/test-hello-window.c

### Implementation for User Story 1

- [X] T019 [P] [US1] Implement HelloApplication application lifecycle methods in src/hello-app/hello-application.c
- [X] T020 [P] [US1] Implement HelloWindow widget creation and setup in src/hello-app/hello-window.c
- [X] T021 [US1] Create GtkBuilder UI template in src/hello-app/resources/hello-window.ui
- [X] T022 [US1] Connect HelloApplication window management in src/hello-app/hello-application.c
- [X] T023 [US1] Implement main() function with GTK initialization in src/hello-app/main.c
- [X] T024 [US1] Add proper signal handling for window close events in src/hello-app/hello-window.c
- [X] T025 [US1] Implement GObject memory management (dispose/finalize) in both classes
- [X] T026 [US1] Add explicit close button to UI template in src/hello-app/resources/hello-window.ui
- [X] T027 [US1] Implement close button signal handler in src/hello-app/hello-window.c
- [X] T028 [US1] Connect close button to application termination sequence

**Checkpoint**: At this point, User Story 1 should be fully functional - launchable hello application with clean termination

---

## Phase 3b: User Story 3 - Image File Selection and Viewing (Priority: P1)

**Goal**: Add image file selection and viewing capabilities with separate viewer windows

**Independent Test**: Launch application, click "Open Image" button, select image files, verify images display in separate windows

### Tests for User Story 3

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [ ] T029 [P] [US3] Unit test for image button functionality in tests/unit/test-hello-window.c
- [ ] T030 [P] [US3] Unit test for image viewer window creation in tests/unit/test-image-viewer.c
- [ ] T031 [P] [US3] Integration test for file chooser dialog in tests/integration/test-file-chooser.py
- [ ] T032 [P] [US3] Integration test for image display functionality in tests/integration/test-image-viewer.py

### Implementation for User Story 3

- [X] T033 [US3] Create HelloImageViewer GObject class foundation in src/hello-app/hello-image-viewer.h
- [X] T034 [US3] Implement HelloImageViewer window structure in src/hello-app/hello-image-viewer.c
- [X] T035 [US3] Add open image button to UI template in src/hello-app/resources/hello-window.ui
- [X] T036 [US3] Implement file chooser dialog with image filters in src/hello-app/hello-window.c
- [ ] T037 [US3] Create image viewer UI template in src/hello-app/resources/hello-image-viewer.ui
- [X] T038 [US3] Implement image loading and display in src/hello-app/hello-image-viewer.c
- [X] T039 [US3] Connect image viewer lifecycle to main window in src/hello-app/hello-window.c
- [X] T040 [US3] Add image format validation and error handling

**Checkpoint**: At this point, User Story 3 should be fully functional - image selection and viewing capabilities

---

## Phase 4: User Story 2 - Cross-Platform Consistency (Priority: P2)

**Goal**: Ensure identical behavior and appearance across Linux, Windows, and macOS platforms

**Independent Test**: Build and run application on all three platforms, verify identical appearance and behavior

### Tests for User Story 2

- [ ] T041 [P] [US2] Platform consistency test for Linux in tests/platform/test-linux.sh
- [ ] T042 [P] [US2] Platform consistency test for Windows in tests/platform/test-windows.sh  
- [ ] T043 [P] [US2] Platform consistency test for macOS in tests/platform/test-macos.sh
- [ ] T044 [P] [US2] Visual regression test for cross-platform UI in tests/integration/test-visual-consistency.py

### Implementation for User Story 2

- [ ] T045 [P] [US2] Configure Linux build environment and packages in docs/setup-linux.md
- [ ] T046 [P] [US2] Configure Windows MSYS2 build environment in docs/setup-windows.md
- [ ] T047 [P] [US2] Configure macOS Homebrew build environment in docs/setup-macos.md
- [ ] T048 [US2] Implement platform-specific build configurations in meson.build
- [ ] T049 [US2] Add GTK theme consistency handling in src/hello-app/hello-window.c
- [ ] T050 [US2] Create cross-platform resource handling in src/hello-app/resources/
- [ ] T051 [US2] Implement platform-specific window management in src/lib/gtk-utils.c
- [ ] T052 [US2] Add cross-platform application packaging configuration

**Checkpoint**: At this point, both User Stories 1 AND 2 should work independently across all platforms

---

## Phase 5: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories and finalize the application

- [ ] T053 [P] Add application icon and desktop integration files in resources/
- [ ] T054 [P] Implement GTK accessibility features in src/hello-app/hello-window.c  
- [ ] T055 [P] Add internationalization support (gettext) in src/hello-app/
- [ ] T056 Code cleanup and GTK coding standards compliance review
- [ ] T057 Performance optimization and memory usage validation
- [ ] T058 [P] Complete unit test coverage in tests/unit/
- [ ] T059 [P] Update documentation in docs/ and README.md
- [ ] T060 Run quickstart.md validation on all platforms

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories  
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2)
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Extends US1 but independently testable

### Within Each User Story

- Tests MUST be written and FAIL before implementation
- GObject class foundations before implementations  
- UI templates before window implementations
- Application lifecycle before window management
- Core functionality before platform-specific enhancements

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, both user stories can start in parallel
- All tests for a user story marked [P] can run in parallel
- Platform-specific tasks within US2 marked [P] can run in parallel
- Documentation tasks in Polish phase marked [P] can run in parallel

---

## Parallel Example: User Story 1

```bash
# Launch all tests for User Story 1 together:
Task: "Unit test for HelloApplication creation in tests/unit/test-hello-application.c"
Task: "Unit test for HelloWindow creation in tests/unit/test-hello-window.c"
Task: "Integration test for application launch in tests/integration/test-application.py"
Task: "Integration test for window display in tests/integration/test-window-display.py"

# Launch all GObject implementations together:
Task: "Implement HelloApplication application lifecycle methods in src/hello-app/hello-application.c"
Task: "Implement HelloWindow widget creation and setup in src/hello-app/hello-window.c"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test User Story 1 independently - should have working hello application
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → GTK foundation ready
2. Add User Story 1 → Test independently → Working hello application (MVP!)
3. Add User Story 2 → Test independently → Cross-platform validated application
4. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (core functionality)
   - Developer B: User Story 2 (platform consistency)
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability  
- Each user story should be independently completable and testable
- Verify tests fail before implementing (TDD approach)
- Follow GTK constitutional requirements throughout
- Use Check framework for C unit testing, DoGTail for GUI integration testing
- Ensure cross-platform compatibility from the start
- Commit after each task or logical group