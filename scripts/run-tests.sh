#!/bin/bash
set -e

echo "🔧 Setting up test environment..."

# Create test output directory
mkdir -p test-results

echo "🧪 Running Unit Tests..."
if [ -d "builddir" ]; then
    echo "  ➤ Running Meson tests..."
    # Set GTK environment for headless testing
    export GDK_BACKEND=x11
    export DISPLAY=${DISPLAY:-:99}
    export GTK_A11Y=none
    export G_MESSAGES_DEBUG=none
    
    meson test -C builddir --verbose --no-stdsplit || {
        echo "❌ Unit tests failed"
        echo "📋 Test log summary:"
        find builddir/meson-logs -name "*test*.txt" -exec echo "=== {} ===" \; -exec cat {} \; 2>/dev/null || true
        exit 1
    }
    echo "  ✅ Unit tests passed"
else
    echo "  ⚠️  Build directory not found. Run 'meson setup builddir' first."
    exit 1
fi

echo "⚡ Running Performance Tests..."
if [ -f "tests/performance/test_performance.py" ]; then
    cd tests/performance
    python3 test_performance.py > ../../test-results/performance.log 2>&1 || {
        echo "❌ Performance tests failed"
        cat ../../test-results/performance.log
        exit 1
    }
    echo "  ✅ Performance tests passed"
    cd ../..
else
    echo "  ⚠️  No performance tests found"
fi

echo "✨ Running Validation Tests..."
if [ -f "tests/validation/test_validation.py" ]; then
    cd tests/validation
    python3 test_validation.py > ../../test-results/validation.log 2>&1 || {
        echo "❌ Validation tests failed"
        cat ../../test-results/validation.log
        exit 1
    }
    echo "  ✅ Validation tests passed"
    cd ../..
else
    echo "  ⚠️  No validation tests found"
fi

echo "🎉 All tests passed successfully!"
echo "📊 Test results saved in test-results/ directory"