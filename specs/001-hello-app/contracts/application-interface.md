# Hello Application Interface Contracts

**Version**: 1.0.0  
**Created**: 2026-02-01  
**Purpose**: Define public interfaces and contracts for GTK Hello Application components

## Application Interface Contract

### HelloApplication Interface

```c
/**
 * HelloApplication:
 * 
 * Main application class extending GtkApplication
 * Manages application lifecycle and single-instance behavior
 */

#define HELLO_TYPE_APPLICATION (hello_application_get_type())
G_DECLARE_FINAL_TYPE(HelloApplication, hello_application, HELLO, APPLICATION, GtkApplication)

/**
 * hello_application_new:
 * 
 * Creates a new HelloApplication instance
 * 
 * Returns: (transfer full): A new HelloApplication instance
 */
HelloApplication *hello_application_new(void);

/**
 * hello_application_get_main_window:
 * @app: A HelloApplication instance
 * 
 * Gets the main window for the application
 * 
 * Returns: (transfer none): The main HelloWindow instance, or NULL if not created
 */
HelloWindow *hello_application_get_main_window(HelloApplication *app);
```

**Contract Specifications**:
- **Application ID**: "com.example.HelloApp" (fixed identifier)
- **Single Instance**: Only one instance allowed per user session
- **Window Management**: Creates and manages one primary window
- **Lifecycle**: Follows GtkApplication standard activation/shutdown pattern

**Error Handling**:
- Graceful handling of multiple launch attempts (show existing window)
- Clean shutdown on all platforms when window is closed
- Proper resource cleanup on application termination

## Window Interface Contract

### HelloWindow Interface

```c
/**
 * HelloWindow:
 * 
 * Main application window class extending GtkApplicationWindow
 * Displays greeting message and handles window lifecycle
 */

#define HELLO_TYPE_WINDOW (hello_window_get_type())
G_DECLARE_FINAL_TYPE(HelloWindow, hello_window, HELLO, WINDOW, GtkApplicationWindow)

/**
 * hello_window_new:
 * @app: The HelloApplication instance
 * 
 * Creates a new HelloWindow instance
 * 
 * Returns: (transfer full): A new HelloWindow instance
 */
HelloWindow *hello_window_new(HelloApplication *app);

/**
 * hello_window_set_greeting:
 * @window: A HelloWindow instance
 * @text: The greeting text to display
 * 
 * Sets the greeting message displayed in the window
 */
void hello_window_set_greeting(HelloWindow *window, const char *text);

/**
 * hello_window_get_greeting:
 * @window: A HelloWindow instance
 * 
 * Gets the current greeting message
 * 
 * Returns: (transfer none): The current greeting text
 */
const char *hello_window_get_greeting(HelloWindow *window);
```

**Contract Specifications**:
- **Default Size**: 400x300 pixels (resizable)
- **Title**: "Hello World" (localized)
- **Content**: Centered greeting label
- **Behavior**: Standard window controls (minimize, maximize, close)

**Window Properties Contract**:
```c
// Window property definitions
enum {
    PROP_GREETING = 1,
    N_PROPS
};

// Default values
#define DEFAULT_WINDOW_WIDTH  400
#define DEFAULT_WINDOW_HEIGHT 300
#define DEFAULT_GREETING_TEXT "Hello World!"
#define DEFAULT_WINDOW_TITLE  "Hello World"
```

**Signal Contract**:
- **destroy**: Emitted when window is being destroyed
- **window-state-event**: Standard GTK window state changes
- **delete-event**: Emitted on close request (allows prevention)

## UI Resource Contracts

### GtkBuilder Template Contract

**Resource Path**: `/com/example/HelloApp/ui/hello-window.ui`

**Template Structure Contract**:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <template class="HelloWindow" parent="GtkApplicationWindow">
    <!-- Window properties -->
    <property name="title" translatable="yes">Hello World</property>
    <property name="default-width">400</property>
    <property name="default-height">300</property>
    <property name="resizable">TRUE</property>
    
    <!-- Window content -->
    <child>
      <object class="GtkLabel" id="greeting_label">
        <property name="label" translatable="yes">Hello World!</property>
        <property name="halign">center</property>
        <property name="valign">center</property>
        <property name="margin-start">20</property>
        <property name="margin-end">20</property>
        <property name="margin-top">20</property>
        <property name="margin-bottom">20</property>
      </object>
    </child>
  </template>
</interface>
```

**Contract Requirements**:
- Must be valid GTK4 UI definition
- Must define template class matching HelloWindow
- Must include internationalization markers (translatable="yes")
- Widget IDs must match expected names in C code

### Resource Bundle Contract

**GResource Definition**: `hello-app.gresource.xml`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<gresources>
  <gresource prefix="/com/example/HelloApp">
    <file>ui/hello-window.ui</file>
    <file>icons/hello-app.png</file>
    <file preprocess="xml-stripblanks">ui/hello-window.ui</file>
  </gresource>
</gresources>
```

**Resource Access Contract**:
- Resources accessible via GResource API
- Embedded in final executable binary
- Cross-platform resource loading

## Build System Contracts

### Meson Build Interface

**Primary Build Configuration**: `meson.build`

```meson
project('hello-app', 'c',
  version: '1.0.0',
  default_options: [
    'warning_level=3',
    'werror=false',
    'c_std=c11',
  ]
)

# GTK4 dependency contract
gtk_dep = dependency('gtk4', version: '>= 4.0.0')

# Application target contract
executable('hello-app',
  sources: [
    'src/main.c',
    'src/hello-application.c', 
    'src/hello-window.c',
    resources
  ],
  dependencies: [gtk_dep],
  install: true
)
```

**Build Requirements**:
- GTK4 version 4.0.0 or newer required
- C11 standard compliance
- Cross-platform build support (Linux, Windows, macOS)

### CMake Alternative Contract

**Alternative Build Configuration**: `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.16)
project(hello-app VERSION 1.0.0 LANGUAGES C)

# GTK4 dependency
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# Executable target
add_executable(hello-app
    src/main.c
    src/hello-application.c
    src/hello-window.c
    ${RESOURCE_FILES}
)

target_link_libraries(hello-app ${GTK4_LIBRARIES})
target_include_directories(hello-app PRIVATE ${GTK4_INCLUDE_DIRS})
```

## Testing Interface Contracts

### Unit Test Interface

**Test Framework Contract**: Using Check framework with GTK testing utilities

```c
// Test suite interface contract
Suite *hello_window_suite(void);
Suite *hello_application_suite(void);

// Individual test contracts
START_TEST(test_hello_window_creation);
START_TEST(test_hello_window_greeting);
START_TEST(test_hello_application_startup);
START_TEST(test_hello_application_singleton);
```

**Test Execution Contract**:
- Tests must be runnable in headless environments
- Cross-platform test execution required
- Automated CI/CD integration support

### Integration Test Interface

**DoGTail Integration Contract**:

```python
# Integration test interface
class HelloAppIntegrationTest:
    def setUp(self): pass
    def tearDown(self): pass
    def test_application_launch(self): pass
    def test_window_display(self): pass
    def test_application_close(self): pass
```

## Platform Compatibility Contracts

### Linux Platform Contract

- **GTK4 Installation**: System packages (libgtk-4-dev)
- **Runtime Dependencies**: GTK4 libraries via package manager
- **Distribution**: AppImage or system packages

### Windows Platform Contract

- **Development Environment**: MSYS2 with MinGW-w64
- **Runtime Dependencies**: Bundled GTK4 libraries
- **Distribution**: NSIS installer with embedded GTK runtime

### macOS Platform Contract

- **Development Environment**: Homebrew GTK4 packages
- **Runtime Dependencies**: Bundled GTK4 frameworks
- **Distribution**: Application bundle (.app) with embedded libraries

## Versioning and Compatibility

**Version Contract**: Semantic versioning (MAJOR.MINOR.PATCH)
- **MAJOR**: Breaking changes to public interfaces
- **MINOR**: New features, backward-compatible changes
- **PATCH**: Bug fixes, no interface changes

**Backward Compatibility**:
- Public C interfaces maintain ABI stability within major versions
- GtkBuilder templates maintain compatibility within major versions
- Resource paths and identifiers stable within major versions

**API Stability Promise**:
- Public function signatures will not change within major versions
- New optional parameters may be added with default values
- Deprecated functions will be marked and documented before removal

This contract specification ensures clear interfaces, proper testing capabilities, and maintainable code structure for the hello application while adhering to GTK development best practices and constitutional requirements.