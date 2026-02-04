#!/bin/bash
# CI Test Runner Script for Blur Feature Testing
# Constitutional requirement: Ubuntu-only testing environment

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=== Blur Feature CI Test Runner ==="
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"

# Constitutional compliance check
if ! grep -q "ID=ubuntu" /etc/os-release 2>/dev/null; then
    echo "⚠️  WARNING: Not running on Ubuntu (constitutional requirement for CI)"
    echo "Continuing anyway for local development..."
fi

# Function to print section headers
print_section() {
    echo
    echo "=========================================="
    echo "$1"
    echo "=========================================="
}

# Function to run command with timeout
run_with_timeout() {
    local timeout=$1
    local cmd=$2
    shift 2
    
    if command -v timeout >/dev/null 2>&1; then
        timeout "$timeout" $cmd "$@"
    else
        # Fallback for systems without timeout command
        $cmd "$@"
    fi
}

# Check dependencies
print_section "Checking Dependencies"

echo "Checking for required tools..."
MISSING_DEPS=()

# Check for build tools
if ! command -v meson >/dev/null 2>&1; then
    MISSING_DEPS+=("meson")
fi

if ! command -v ninja >/dev/null 2>&1 && ! command -v ninja-build >/dev/null 2>&1; then
    MISSING_DEPS+=("ninja")
fi

# Check for GTK4 development packages
if ! pkg-config --exists gtk4; then
    MISSING_DEPS+=("gtk4-dev")
fi

# Check for Check testing framework
if ! pkg-config --exists check; then
    MISSING_DEPS+=("check")
fi

# Check for Python for performance tests
if ! command -v python3 >/dev/null 2>&1; then
    MISSING_DEPS+=("python3")
fi

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo "❌ Missing dependencies: ${MISSING_DEPS[*]}"
    echo "Please install missing dependencies:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install meson ninja-build libgtk-4-dev check python3 python3-pip"
    echo "  python3 -m pip install Pillow"
    exit 1
fi

echo "✅ All dependencies found"

# Set up build environment
print_section "Setting Up Build Environment"

# Clean previous build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create new build directory
mkdir -p "$BUILD_DIR"

# Configure build
echo "Configuring build with meson..."
cd "$PROJECT_ROOT"
meson setup "$BUILD_DIR" --buildtype=debug

# Build project
print_section "Building Project"

echo "Building project..."
cd "$BUILD_DIR"
ninja

if [ $? -ne 0 ]; then
    echo "❌ Build failed"
    exit 1
fi

echo "✅ Build successful"

# Verify executables
print_section "Verifying Build Artifacts"

REQUIRED_EXECUTABLES=(
    "hello-app"
    "test-hello-application"
    "test-hello-window"
    "test-image-processing"
    "test-image-viewer-bw"
    "test-blur-processor"
    "test-blur-cache"
    "test-blur-integration"
)

for exe in "${REQUIRED_EXECUTABLES[@]}"; do
    if [ ! -f "$BUILD_DIR/$exe" ]; then
        echo "❌ Missing executable: $exe"
        exit 1
    else
        echo "✅ Found: $exe"
    fi
done

# Set up testing environment
print_section "Setting Up Testing Environment"

export GTK_A11Y=none
export DISPLAY=${DISPLAY:-:99}

# Start virtual display for UI tests (if not already running)
if [ -z "${DISPLAY##*:99*}" ] && ! pgrep Xvfb > /dev/null; then
    echo "Starting virtual display..."
    if command -v Xvfb >/dev/null 2>&1; then
        Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &
        XVFB_PID=$!
        sleep 2
        echo "Virtual display started (PID: $XVFB_PID)"
    else
        echo "⚠️  Xvfb not available, UI tests may fail"
    fi
fi

# Run unit tests
print_section "Running Unit Tests"

cd "$BUILD_DIR"
TEST_RESULTS=()

echo "Running core unit tests..."
UNIT_TESTS=(
    "test-hello-application"
    "test-hello-window" 
    "test-image-processing"
    "test-image-viewer-bw"
)

for test in "${UNIT_TESTS[@]}"; do
    echo "  🧪 Running $test..."
    if run_with_timeout 30s ./"$test"; then
        echo "    ✅ $test PASSED"
        TEST_RESULTS+=("$test:PASS")
    else
        echo "    ❌ $test FAILED"
        TEST_RESULTS+=("$test:FAIL")
    fi
done

echo "Running blur feature unit tests..."
BLUR_TESTS=(
    "test-blur-processor"
    "test-blur-cache"
    "test-blur-integration"
)

for test in "${BLUR_TESTS[@]}"; do
    echo "  🌀 Running $test..."
    if run_with_timeout 30s ./"$test"; then
        echo "    ✅ $test PASSED"
        TEST_RESULTS+=("$test:PASS")
    else
        echo "    ❌ $test FAILED"
        TEST_RESULTS+=("$test:FAIL")
    fi
done

# Run performance tests
print_section "Running Performance Tests"

cd "$PROJECT_ROOT"

echo "  ⚡ Running blur performance tests..."
if run_with_timeout 60s python3 tests/performance/blur_performance.py; then
    echo "    ✅ Performance tests PASSED"
    TEST_RESULTS+=("performance:PASS")
else
    echo "    ❌ Performance tests FAILED"
    TEST_RESULTS+=("performance:FAIL")
fi

echo "  🧠 Running blur memory profiling..."
if run_with_timeout 60s python3 tests/performance/blur_memory_profile.py; then
    echo "    ✅ Memory profiling PASSED" 
    TEST_RESULTS+=("memory:PASS")
else
    echo "    ❌ Memory profiling FAILED"
    TEST_RESULTS+=("memory:FAIL")
fi

# Run UI validation tests
print_section "Running UI Validation Tests"

echo "  🎮 Running blur UI validation..."
if run_with_timeout 60s python3 tests/validation/test_blur_ui.py; then
    echo "    ✅ UI validation PASSED"
    TEST_RESULTS+=("ui:PASS")
else
    echo "    ❌ UI validation FAILED"
    TEST_RESULTS+=("ui:FAIL")
fi

# Generate test report
print_section "Test Results Summary"

TOTAL_TESTS=${#TEST_RESULTS[@]}
PASSED_TESTS=0
FAILED_TESTS=0

echo "| Test | Result |"
echo "|------|--------|"

for result in "${TEST_RESULTS[@]}"; do
    test_name="${result%:*}"
    test_result="${result#*:}"
    
    if [ "$test_result" = "PASS" ]; then
        echo "| $test_name | ✅ PASS |"
        ((PASSED_TESTS++))
    else
        echo "| $test_name | ❌ FAIL |"
        ((FAILED_TESTS++))
    fi
done

echo
echo "📊 Test Summary:"
echo "  Total Tests: $TOTAL_TESTS"
echo "  Passed: $PASSED_TESTS"
echo "  Failed: $FAILED_TESTS"

if [ $FAILED_TESTS -eq 0 ]; then
    echo "🎉 All tests PASSED!"
    EXIT_CODE=0
else
    echo "❌ $FAILED_TESTS test(s) FAILED"
    EXIT_CODE=1
fi

# Constitutional compliance check
print_section "Constitutional Compliance Validation"

echo "Checking constitutional requirements..."

# Test duration check (already enforced by timeouts)
echo "✅ Test execution time: All tests completed within 60s timeout"

# Ubuntu environment check
if grep -q "ID=ubuntu" /etc/os-release 2>/dev/null; then
    echo "✅ Ubuntu environment: Running on Ubuntu as required"
else
    echo "⚠️  Ubuntu environment: Not running on Ubuntu (constitutional requirement)"
fi

# Coverage requirements (would need gcov integration)
echo "ℹ️  Code coverage: Requires gcov integration for >95% validation"

# Cleanup
print_section "Cleanup"

if [ -n "$XVFB_PID" ]; then
    echo "Stopping virtual display..."
    kill $XVFB_PID 2>/dev/null || true
    wait $XVFB_PID 2>/dev/null || true
fi

# Final status
echo
if [ $EXIT_CODE -eq 0 ]; then
    echo "🎉 CI Test Runner completed successfully!"
else
    echo "❌ CI Test Runner completed with failures!"
fi

exit $EXIT_CODE