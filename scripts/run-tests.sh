#!/bin/bash
set -e

echo "🔧 Setting up test environment..."

# Create test output directory
mkdir -p test-results

echo "🧪 Running Unit Tests..."
for test_file in tests/unit/test-*.c; do
    if [ -f "$test_file" ]; then
        test_name=$(basename "$test_file" .c)
        echo "  ➤ Compiling $test_name..."
        
        gcc "$test_file" src/lib/image-processing.c \
            $(pkg-config --cflags --libs gtk4) \
            -o "builddir/$test_name" \
            -DTEST_DATA_DIR=\"$(pwd)/tests/data\" || {
            echo "❌ Failed to compile $test_name"
            exit 1
        }
        
        echo "  ➤ Running $test_name..."
        "./builddir/$test_name" > "test-results/$test_name.log" 2>&1 || {
            echo "❌ $test_name failed"
            cat "test-results/$test_name.log"
            exit 1
        }
        echo "  ✅ $test_name passed"
    fi
done

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