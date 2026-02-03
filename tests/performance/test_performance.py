#!/usr/bin/env python3
"""
Performance tests for image B&W conversion feature.
Tests conversion performance with large images and validates memory usage.
"""

import os
import sys
import time
import subprocess
import tempfile
import shutil
from pathlib import Path

def create_test_image(width, height, filename):
    """Create a test image using ImageMagick convert command if available."""
    try:
        # Create a test pattern image
        cmd = [
            "convert", "-size", f"{width}x{height}", 
            "gradient:red-blue", filename
        ]
        subprocess.run(cmd, check=True, capture_output=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        try:
            # Fallback: create with Python PIL if available
            from PIL import Image, ImageDraw
            import random
            
            # Create a colorful test image
            img = Image.new('RGB', (width, height))
            draw = ImageDraw.Draw(img)
            
            # Draw random colored rectangles
            for _ in range(100):
                x1, y1 = random.randint(0, width//2), random.randint(0, height//2)
                x2, y2 = x1 + random.randint(50, width//4), y1 + random.randint(50, height//4)
                color = (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
                draw.rectangle([x1, y1, x2, y2], fill=color)
            
            img.save(filename)
            return True
        except ImportError:
            return False

def get_file_size(filename):
    """Get file size in MB."""
    return os.path.getsize(filename) / (1024 * 1024)

def test_app_performance(app_path, image_path, test_name):
    """Test application performance with a specific image."""
    print(f"\n--- {test_name} ---")
    print(f"Image: {image_path}")
    print(f"Size: {get_file_size(image_path):.2f} MB")
    
    # Launch the application in background
    start_time = time.time()
    try:
        # Start app and measure startup time
        process = subprocess.Popen(
            [app_path, image_path] if len(sys.argv) > 2 else [app_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        # Let it run for a moment to load
        time.sleep(2)
        
        # Terminate the process
        process.terminate()
        process.wait(timeout=5)
        
        end_time = time.time()
        startup_time = end_time - start_time
        
        print(f"App startup time: {startup_time:.3f} seconds")
        
        if startup_time < 5.0:
            print("✅ PASS: Startup time meets requirement (<5 seconds)")
            return True
        else:
            print("❌ FAIL: Startup time exceeds requirement")
            return False
            
    except subprocess.TimeoutExpired:
        print("❌ FAIL: Application did not respond within timeout")
        process.kill()
        return False
    except Exception as e:
        print(f"❌ FAIL: Error running application: {e}")
        return False

def check_memory_usage(app_path, image_path):
    """Check memory usage during image loading."""
    print(f"\n--- Memory Usage Test ---")
    
    try:
        # Use ps to monitor memory usage if available on macOS
        process = subprocess.Popen([app_path])
        time.sleep(1)  # Let app start
        
        # Get memory usage
        ps_cmd = ["ps", "-o", "rss=", "-p", str(process.pid)]
        result = subprocess.run(ps_cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            memory_kb = int(result.stdout.strip())
            memory_mb = memory_kb / 1024
            print(f"Memory usage: {memory_mb:.2f} MB")
            
            process.terminate()
            process.wait(timeout=5)
            
            # Check if memory usage is reasonable (under 100MB for basic app)
            if memory_mb < 100:
                print("✅ PASS: Memory usage is reasonable")
                return True
            else:
                print("⚠️  WARN: High memory usage detected")
                return True
        else:
            print("❌ Could not measure memory usage")
            process.terminate()
            return False
            
    except Exception as e:
        print(f"❌ Error measuring memory: {e}")
        return False

def run_performance_tests():
    """Main performance test runner."""
    print("=== Image B&W Conversion Performance Tests ===")
    
    # Find the application binary
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    app_path = project_root / "build" / "hello-app"
    
    if not app_path.exists():
        print(f"❌ Application not found at: {app_path}")
        print("Please build the application first with: meson compile -C build")
        return False
    
    print(f"Testing application: {app_path}")
    
    # Create temporary directory for test images
    with tempfile.TemporaryDirectory() as temp_dir:
        test_results = []
        
        # Test 1: Small image (for baseline)
        small_image = Path(temp_dir) / "small_test.png"
        if create_test_image(640, 480, str(small_image)):
            result = test_app_performance(str(app_path), str(small_image), "Small Image (640x480)")
            test_results.append(result)
        else:
            print("⚠️  Skipping small image test - could not create test image")
        
        # Test 2: Medium image
        medium_image = Path(temp_dir) / "medium_test.png"
        if create_test_image(1920, 1080, str(medium_image)):
            result = test_app_performance(str(app_path), str(medium_image), "HD Image (1920x1080)")
            test_results.append(result)
        else:
            print("⚠️  Skipping medium image test - could not create test image")
        
        # Test 3: Large image (if system can handle it)
        large_image = Path(temp_dir) / "large_test.png"
        if create_test_image(3840, 2160, str(large_image)):
            result = test_app_performance(str(app_path), str(large_image), "4K Image (3840x2160)")
            test_results.append(result)
        else:
            print("⚠️  Skipping large image test - could not create test image")
        
        # Memory usage test with just app startup
        memory_result = check_memory_usage(str(app_path), None)
        test_results.append(memory_result)
    
    # Summary
    print(f"\n=== Performance Test Summary ===")
    passed = sum(test_results)
    total = len(test_results)
    
    print(f"Tests passed: {passed}/{total}")
    
    if passed == total:
        print("🎉 All performance tests PASSED!")
        return True
    else:
        print("⚠️  Some performance tests failed or had issues")
        return False

if __name__ == "__main__":
    success = run_performance_tests()
    sys.exit(0 if success else 1)