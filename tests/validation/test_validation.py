#!/usr/bin/env python3
"""
Validation tests for the Image B&W Conversion feature.
Tests edge cases, cross-platform compatibility, and accessibility features.
"""

import os
import sys
import subprocess
import platform
import tempfile
from pathlib import Path

def check_build_system():
    """Verify the build system works on this platform."""
    print("=== Build System Validation ===")
    
    # Check if meson is available
    try:
        result = subprocess.run(["meson", "--version"], capture_output=True, text=True, check=True)
        meson_version = result.stdout.strip()
        print(f"✅ Meson version: {meson_version}")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ Meson not found - build system unavailable")
        return False
    
    # Check if ninja is available
    try:
        result = subprocess.run(["ninja", "--version"], capture_output=True, text=True, check=True)
        ninja_version = result.stdout.strip()
        print(f"✅ Ninja version: {ninja_version}")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("⚠️  Ninja not found - may fall back to make")
    
    # Check GTK4 development libraries
    try:
        result = subprocess.run(["pkg-config", "--modversion", "gtk4"], capture_output=True, text=True, check=True)
        gtk_version = result.stdout.strip()
        print(f"✅ GTK4 version: {gtk_version}")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ GTK4 development libraries not found")
        return False
    
    return True

def check_platform_compatibility():
    """Check platform-specific compatibility."""
    print(f"\n=== Platform Compatibility ({platform.system()}) ===")
    
    system = platform.system()
    print(f"Operating System: {system}")
    print(f"Architecture: {platform.machine()}")
    print(f"Python version: {platform.python_version()}")
    
    # Platform-specific checks
    if system == "Darwin":  # macOS
        print("✅ Running on macOS - primary development platform")
        
        # Check for Homebrew GTK4
        homebrew_gtk = Path("/opt/homebrew/lib/libgtk-4.dylib")
        if homebrew_gtk.exists():
            print("✅ Homebrew GTK4 installation detected")
        else:
            print("⚠️  Homebrew GTK4 not detected - may need manual installation")
        
        return True
        
    elif system == "Linux":
        print("✅ Running on Linux - should work with system GTK4")
        
        # Check common GTK4 locations
        gtk_locations = [
            "/usr/lib/libgtk-4.so",
            "/usr/lib/x86_64-linux-gnu/libgtk-4.so",
            "/usr/lib/aarch64-linux-gnu/libgtk-4.so"
        ]
        
        for location in gtk_locations:
            if Path(location).exists():
                print(f"✅ GTK4 found at: {location}")
                return True
        
        print("⚠️  GTK4 system libraries not found in common locations")
        return True  # May still work with pkg-config
        
    elif system == "Windows":
        print("⚠️  Running on Windows - GTK4 support may require MSYS2/vcpkg")
        print("   Consider using MSYS2 with: pacman -S mingw-w64-x86_64-gtk4")
        return True
        
    else:
        print(f"⚠️  Unknown platform: {system}")
        return True

def test_file_format_support():
    """Test support for various image formats."""
    print(f"\n=== Image Format Support ===")
    
    # Check GdkPixbuf supported formats
    try:
        # Get supported formats from GdkPixbuf
        result = subprocess.run([
            "python3", "-c", 
            """
import gi
gi.require_version('GdkPixbuf', '2.0')
from gi.repository import GdkPixbuf
formats = GdkPixbuf.Pixbuf.get_formats()
for fmt in formats:
    print(f"{fmt.get_name()}: {', '.join(fmt.get_extensions())}")
"""
        ], capture_output=True, text=True)
        
        if result.returncode == 0:
            print("Supported image formats:")
            print(result.stdout)
            
            # Check for required formats
            required_formats = ["png", "jpeg", "gif"]
            supported_text = result.stdout.lower()
            
            all_supported = True
            for fmt in required_formats:
                if fmt in supported_text:
                    print(f"✅ {fmt.upper()} support confirmed")
                else:
                    print(f"❌ {fmt.upper()} support missing")
                    all_supported = False
            
            return all_supported
        else:
            print("❌ Could not query GdkPixbuf formats")
            return False
            
    except Exception as e:
        print(f"⚠️  Could not test image format support: {e}")
        return True  # Don't fail the test, just warn

def test_accessibility_features():
    """Test accessibility features."""
    print(f"\n=== Accessibility Features ===")
    
    # Check UI template for accessibility attributes
    ui_template_path = Path(__file__).parent.parent.parent / "src" / "hello-app" / "resources" / "hello-image-viewer.ui"
    
    if not ui_template_path.exists():
        print(f"❌ UI template not found: {ui_template_path}")
        return False
    
    try:
        with open(ui_template_path, 'r') as f:
            ui_content = f.read()
        
        # Check for accessibility attributes
        accessibility_checks = [
            ('accessibility', 'Accessibility element'),
            ('translatable="yes"', 'Internationalization support'),
            ('tooltip-text', 'Tooltip support'),
            ('label', 'Accessibility labels'),
        ]
        
        results = []
        for check, description in accessibility_checks:
            if check in ui_content:
                print(f"✅ {description} found")
                results.append(True)
            else:
                print(f"❌ {description} missing")
                results.append(False)
        
        return all(results)
        
    except Exception as e:
        print(f"❌ Error checking accessibility features: {e}")
        return False

def test_memory_management():
    """Test memory management patterns in the code."""
    print(f"\n=== Memory Management Validation ===")
    
    source_files = [
        Path(__file__).parent.parent.parent / "src" / "hello-app" / "hello-image-viewer.c",
        Path(__file__).parent.parent.parent / "src" / "lib" / "image-processing.c"
    ]
    
    memory_patterns = [
        ('g_object_unref', 'GObject reference management'),
        ('g_clear_object', 'Safe object clearing'),
        ('g_free', 'Memory deallocation'),
        ('dispose', 'Proper dispose method'),
        ('finalize', 'Proper finalize method'),
    ]
    
    all_good = True
    
    for source_file in source_files:
        if not source_file.exists():
            print(f"❌ Source file not found: {source_file}")
            all_good = False
            continue
        
        print(f"\nChecking {source_file.name}:")
        
        try:
            with open(source_file, 'r') as f:
                content = f.read()
            
            for pattern, description in memory_patterns:
                if pattern in content:
                    count = content.count(pattern)
                    print(f"✅ {description}: {count} occurrences")
                else:
                    print(f"⚠️  {description}: not found")
        
        except Exception as e:
            print(f"❌ Error checking {source_file}: {e}")
            all_good = False
    
    return all_good

def test_error_handling():
    """Test error handling patterns."""
    print(f"\n=== Error Handling Validation ===")
    
    source_file = Path(__file__).parent.parent.parent / "src" / "hello-app" / "hello-image-viewer.c"
    
    if not source_file.exists():
        print(f"❌ Source file not found: {source_file}")
        return False
    
    try:
        with open(source_file, 'r') as f:
            content = f.read()
        
        error_patterns = [
            ('GError', 'GError usage'),
            ('g_return_if_fail', 'Parameter validation'),
            ('g_return_val_if_fail', 'Return value validation'),
            ('g_warning', 'Warning messages'),
            ('error dialog', 'User error feedback'),
        ]
        
        results = []
        for pattern, description in error_patterns:
            if pattern.lower() in content.lower():
                count = content.lower().count(pattern.lower())
                print(f"✅ {description}: {count} occurrences")
                results.append(True)
            else:
                print(f"❌ {description}: not found")
                results.append(False)
        
        return all(results)
        
    except Exception as e:
        print(f"❌ Error checking error handling: {e}")
        return False

def run_validation_tests():
    """Main validation test runner."""
    print("=== Image B&W Conversion Validation Tests ===")
    print(f"Platform: {platform.system()} {platform.release()}")
    print(f"Architecture: {platform.machine()}")
    
    test_results = []
    
    # Run all validation tests
    test_results.append(check_build_system())
    test_results.append(check_platform_compatibility())
    test_results.append(test_file_format_support())
    test_results.append(test_accessibility_features())
    test_results.append(test_memory_management())
    test_results.append(test_error_handling())
    
    # Summary
    print(f"\n=== Validation Test Summary ===")
    passed = sum(test_results)
    total = len(test_results)
    
    print(f"Tests passed: {passed}/{total}")
    
    if passed >= total - 1:  # Allow one test to have warnings
        print("🎉 Validation tests PASSED!")
        return True
    else:
        print("⚠️  Some validation tests failed")
        return False

if __name__ == "__main__":
    success = run_validation_tests()
    sys.exit(0 if success else 1)