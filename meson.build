project('hello-app', 'c',
  version: '1.0.0',
  license: 'MIT',
  default_options: [
    'warning_level=3',
    'c_std=c11',
    'werror=false'
  ]
)

# Dependencies
gtk_dep = dependency('gtk4', version: '>= 4.0.0')
math_dep = meson.get_compiler('c').find_library('m', required: false)
check_dep = dependency('check', required: false)

# Generate config.h
config_data = configuration_data()
config_data.set_quoted('VERSION', meson.project_version())
config_data.set_quoted('APPLICATION_ID', 'com.example.HelloApp')
configure_file(
  output: 'config.h',
  configuration: config_data
)

# Include directories
inc = include_directories('src', '.')

# GResource compilation
gnome = import('gnome')
resources = gnome.compile_resources(
  'hello-resources',
  'src/hello-app/hello-app.gresource.xml',
  source_dir: 'src/hello-app',
  c_name: 'hello_resources'
)

# Library for reusable GTK utilities
gtk_utils_lib = static_library('gtk-utils',
  'src/lib/gtk-utils.c',
  dependencies: [gtk_dep],
  include_directories: inc
)

# Image processing library for B&W conversion
image_processing_lib = static_library('image-processing',
  'src/lib/image-processing.c',
  dependencies: [gtk_dep],
  include_directories: inc
)

# Blur processing library for Gaussian blur effects
blur_processor_lib = static_library('blur-processor',
  'src/lib/blur-processor.c',
  dependencies: [gtk_dep, math_dep],
  include_directories: inc
)

# Blur cache library for LRU caching of blur results
blur_cache_lib = static_library('blur-cache',
  'src/lib/blur-cache.c',
  dependencies: [gtk_dep],
  include_directories: inc
)

# Main application executable
hello_app = executable('hello-app',
  [
    'src/hello-app/main.c',
    'src/hello-app/hello-application.c',
    'src/hello-app/hello-window.c',
    'src/hello-app/hello-image-viewer.c',
    resources,
  ],
  dependencies: [gtk_dep],
  link_with: [gtk_utils_lib, image_processing_lib, blur_processor_lib, blur_cache_lib],
  include_directories: inc,
  install: true
)

# Unit tests (if Check framework is available)
if check_dep.found()
  test_hello_application = executable('test-hello-application',
    ['tests/unit/test-hello-application.c', 'src/hello-app/hello-application.c', 'src/hello-app/hello-window.c', 'src/hello-app/hello-image-viewer.c', resources],
    dependencies: [gtk_dep, check_dep, math_dep],
    link_with: [gtk_utils_lib, image_processing_lib, blur_processor_lib, blur_cache_lib],
    include_directories: inc
  )

  test_hello_window = executable('test-hello-window',
    ['tests/unit/test-hello-window.c', 'src/hello-app/hello-window.c', 'src/hello-app/hello-application.c', 'src/hello-app/hello-image-viewer.c', resources],
    dependencies: [gtk_dep, check_dep, math_dep],
    link_with: [gtk_utils_lib, image_processing_lib, blur_processor_lib, blur_cache_lib],
    include_directories: inc
  )

  test_image_processing = executable('test-image-processing',
    'tests/unit/test-image-processing.c',
    dependencies: [gtk_dep, check_dep],
    link_with: [image_processing_lib],
    include_directories: inc
  )

  test_image_viewer_bw = executable('test-image-viewer-bw',
    ['tests/unit/test-image-viewer-bw.c', 'src/hello-app/hello-image-viewer.c', 'src/hello-app/hello-window.c', 'src/hello-app/hello-application.c', resources],
    dependencies: [gtk_dep, check_dep, math_dep],
    link_with: [gtk_utils_lib, image_processing_lib, blur_processor_lib, blur_cache_lib],
    include_directories: inc
  )

  # Blur feature unit tests
  test_blur_processor = executable('test-blur-processor',
    'tests/unit/test-blur-processor.c',
    dependencies: [gtk_dep, check_dep, math_dep],
    link_with: [blur_processor_lib],
    include_directories: inc
  )

  test_blur_cache = executable('test-blur-cache',
    'tests/unit/test-blur-cache.c',
    dependencies: [gtk_dep, check_dep],
    link_with: [blur_cache_lib],
    include_directories: inc
  )

  test_blur_integration = executable('test-blur-integration',
    'tests/unit/test-blur-integration.c',
    dependencies: [gtk_dep, check_dep, math_dep],
    link_with: [blur_processor_lib, blur_cache_lib],
    include_directories: inc
  )

  test('test-hello-application', test_hello_application)
  test('test-hello-window', test_hello_window)
  test('test-image-processing', test_image_processing)
  test('test-image-viewer-bw', test_image_viewer_bw)
  test('test-blur-processor', test_blur_processor)
  test('test-blur-cache', test_blur_cache)
  test('test-blur-integration', test_blur_integration)
endif

# Install desktop file and icon (optional for later phases)
if get_option('install_desktop_files')
  install_data('data/hello-app.desktop',
    install_dir: join_paths(get_option('datadir'), 'applications')
  )
endif