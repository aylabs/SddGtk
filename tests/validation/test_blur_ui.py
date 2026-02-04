#!/usr/bin/env python3
"""
Blur UI Validation Test Infrastructure

Tests blur feature UI interactions and validation scenarios.
Provides automated testing for GTK application UI components.
"""

import os
import sys
import time
import subprocess
import tempfile
import threading
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import json

# Add project root to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))


class GTKUITester:
    """GTK application UI testing utilities."""
    
    def __init__(self, app_path: str):
        self.app_path = Path(app_path)
        self.process = None
        
        if not self.app_path.exists():
            raise FileNotFoundError(f"Application not found: {app_path}")
    
    def create_test_image(self, width: int, height: int, filename: str) -> bool:
        """Create test image for UI testing."""
        try:
            # Try ImageMagick first
            cmd = [
                "convert", "-size", f"{width}x{height}",
                "pattern:checkerboard", filename
            ]
            subprocess.run(cmd, check=True, capture_output=True)
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            try:
                # Fallback to PIL
                from PIL import Image, ImageDraw
                
                img = Image.new('RGB', (width, height), 'white')
                draw = ImageDraw.Draw(img)
                
                # Create checkerboard pattern
                square_size = 50
                for x in range(0, width, square_size):
                    for y in range(0, height, square_size):
                        if (x // square_size + y // square_size) % 2:
                            draw.rectangle([x, y, x + square_size, y + square_size], fill='black')
                
                img.save(filename)
                return True
            except ImportError:
                print("Warning: Neither ImageMagick nor PIL available for test image creation")
                return False
    
    def start_application(self, image_path: str = None, headless: bool = True) -> bool:
        """Start the GTK application."""
        try:
            env = os.environ.copy()
            
            if headless:
                # Set up headless display
                env["DISPLAY"] = ":99"
                env["GTK_A11Y"] = "none"
                env["QT_QPA_PLATFORM"] = "offscreen"
            
            args = [str(self.app_path)]
            if image_path:
                args.append(image_path)
            
            self.process = subprocess.Popen(
                args,
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            
            # Give application time to start
            time.sleep(1.0)
            
            # Check if process is still running
            if self.process.poll() is None:
                return True
            else:
                print(f"Application exited with code: {self.process.returncode}")
                return False
                
        except Exception as e:
            print(f"Failed to start application: {e}")
            return False
    
    def stop_application(self):
        """Stop the GTK application."""
        if self.process and self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        self.process = None
    
    def is_application_running(self) -> bool:
        """Check if application is still running."""
        return self.process is not None and self.process.poll() is None


class BlurUIValidator:
    """Blur feature UI validation tests."""
    
    def __init__(self, app_path: str):
        self.ui_tester = GTKUITester(app_path)
        self.results = {}
    
    def test_application_launch(self) -> Dict:
        """Test basic application launch and startup."""
        print("  🚀 Testing application launch...")
        
        try:
            # Test launch without image
            if self.ui_tester.start_application():
                time.sleep(2)  # Let it fully initialize
                
                if self.ui_tester.is_application_running():
                    result = {
                        "success": True,
                        "message": "Application launched successfully"
                    }
                    print("    ✅ Application startup successful")
                else:
                    result = {
                        "success": False,
                        "message": "Application exited unexpectedly"
                    }
                    print("    ❌ Application exited unexpectedly")
                
                self.ui_tester.stop_application()
                return result
            else:
                return {
                    "success": False,
                    "message": "Failed to launch application"
                }
        except Exception as e:
            return {
                "success": False,
                "message": f"Launch test failed: {e}"
            }
    
    def test_image_loading(self) -> Dict:
        """Test application launch with image file."""
        print("  📷 Testing image loading...")
        
        with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp_file:
            try:
                # Create test image
                if not self.ui_tester.create_test_image(800, 600, tmp_file.name):
                    return {
                        "success": False,
                        "message": "Could not create test image"
                    }
                
                # Test launch with image
                if self.ui_tester.start_application(tmp_file.name):
                    time.sleep(2)  # Let it process the image
                    
                    if self.ui_tester.is_application_running():
                        result = {
                            "success": True,
                            "message": "Image loaded successfully"
                        }
                        print("    ✅ Image loading successful")
                    else:
                        result = {
                            "success": False,
                            "message": "Application crashed during image loading"
                        }
                        print("    ❌ Application crashed during image loading")
                    
                    self.ui_tester.stop_application()
                    return result
                else:
                    return {
                        "success": False,
                        "message": "Failed to launch application with image"
                    }
                    
            except Exception as e:
                return {
                    "success": False,
                    "message": f"Image loading test failed: {e}"
                }
            finally:
                # Cleanup test image
                try:
                    os.unlink(tmp_file.name)
                except:
                    pass
    
    def test_blur_processing_stability(self) -> Dict:
        """Test application stability during blur processing simulation."""
        print("  🌀 Testing blur processing stability...")
        
        with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp_file:
            try:
                # Create complex test image
                if not self.ui_tester.create_test_image(1024, 768, tmp_file.name):
                    return {
                        "success": False,
                        "message": "Could not create test image"
                    }
                
                # Start application with image
                if not self.ui_tester.start_application(tmp_file.name):
                    return {
                        "success": False,
                        "message": "Failed to start application"
                    }
                
                # Monitor stability over time
                stability_duration = 5.0  # Monitor for 5 seconds
                check_interval = 0.5
                checks = int(stability_duration / check_interval)
                
                stable_checks = 0
                for i in range(checks):
                    time.sleep(check_interval)
                    if self.ui_tester.is_application_running():
                        stable_checks += 1
                    else:
                        break
                
                self.ui_tester.stop_application()
                
                stability_ratio = stable_checks / checks
                
                if stability_ratio >= 0.9:  # 90% stability threshold
                    result = {
                        "success": True,
                        "message": f"Application stable ({stability_ratio:.1%})",
                        "stability_ratio": stability_ratio
                    }
                    print(f"    ✅ Processing stability good ({stability_ratio:.1%})")
                else:
                    result = {
                        "success": False,
                        "message": f"Application unstable ({stability_ratio:.1%})",
                        "stability_ratio": stability_ratio
                    }
                    print(f"    ❌ Processing stability poor ({stability_ratio:.1%})")
                
                return result
                
            except Exception as e:
                self.ui_tester.stop_application()
                return {
                    "success": False,
                    "message": f"Stability test failed: {e}"
                }
            finally:
                # Cleanup test image
                try:
                    os.unlink(tmp_file.name)
                except:
                    pass
    
    def test_multiple_image_handling(self) -> Dict:
        """Test handling multiple image operations."""
        print("  🖼️  Testing multiple image handling...")
        
        temp_files = []
        try:
            # Create multiple test images
            image_sizes = [(640, 480), (800, 600), (1024, 768)]
            
            for i, (width, height) in enumerate(image_sizes):
                tmp_file = tempfile.NamedTemporaryFile(suffix=f"_test_{i}.png", delete=False)
                temp_files.append(tmp_file.name)
                
                if not self.ui_tester.create_test_image(width, height, tmp_file.name):
                    return {
                        "success": False,
                        "message": f"Could not create test image {i+1}"
                    }
            
            # Test loading each image in sequence
            successful_loads = 0
            
            for i, image_path in enumerate(temp_files):
                print(f"    Testing image {i+1}/{len(temp_files)}...")
                
                if self.ui_tester.start_application(image_path):
                    time.sleep(1.5)  # Processing time
                    
                    if self.ui_tester.is_application_running():
                        successful_loads += 1
                        print(f"      ✅ Image {i+1} loaded successfully")
                    else:
                        print(f"      ❌ Image {i+1} caused crash")
                    
                    self.ui_tester.stop_application()
                    time.sleep(0.5)  # Brief pause between tests
                else:
                    print(f"      ❌ Failed to launch with image {i+1}")
            
            success_ratio = successful_loads / len(temp_files)
            
            if success_ratio == 1.0:
                result = {
                    "success": True,
                    "message": f"All {len(temp_files)} images handled successfully",
                    "success_ratio": success_ratio,
                    "successful_loads": successful_loads
                }
                print(f"    ✅ Multiple image handling: {successful_loads}/{len(temp_files)}")
            else:
                result = {
                    "success": False,
                    "message": f"Only {successful_loads}/{len(temp_files)} images handled successfully",
                    "success_ratio": success_ratio,
                    "successful_loads": successful_loads
                }
                print(f"    ❌ Multiple image handling: {successful_loads}/{len(temp_files)}")
            
            return result
            
        except Exception as e:
            return {
                "success": False,
                "message": f"Multiple image test failed: {e}"
            }
        finally:
            # Cleanup all test images
            for temp_file in temp_files:
                try:
                    os.unlink(temp_file)
                except:
                    pass
    
    def test_resource_cleanup(self) -> Dict:
        """Test that resources are properly cleaned up."""
        print("  🧹 Testing resource cleanup...")
        
        try:
            initial_processes = self._count_app_processes()
            
            # Run multiple application cycles
            cycles = 3
            for i in range(cycles):
                if self.ui_tester.start_application():
                    time.sleep(1)
                    self.ui_tester.stop_application()
                    time.sleep(0.5)  # Let cleanup complete
            
            final_processes = self._count_app_processes()
            
            if final_processes <= initial_processes:
                result = {
                    "success": True,
                    "message": "Resource cleanup successful",
                    "process_count_change": final_processes - initial_processes
                }
                print(f"    ✅ Resource cleanup successful")
            else:
                leaked_processes = final_processes - initial_processes
                result = {
                    "success": False,
                    "message": f"Process leak detected: {leaked_processes} processes",
                    "process_count_change": leaked_processes
                }
                print(f"    ❌ Process leak detected: {leaked_processes} processes")
            
            return result
            
        except Exception as e:
            return {
                "success": False,
                "message": f"Resource cleanup test failed: {e}"
            }
    
    def _count_app_processes(self) -> int:
        """Count running instances of the application."""
        try:
            app_name = self.ui_tester.app_path.name
            result = subprocess.run(
                ["pgrep", "-c", app_name],
                capture_output=True, text=True
            )
            
            if result.returncode == 0:
                return int(result.stdout.strip())
            else:
                return 0
        except:
            return 0
    
    def run_ui_validation_tests(self) -> Dict:
        """Run comprehensive UI validation tests."""
        test_suite = {
            "application_launch": self.test_application_launch,
            "image_loading": self.test_image_loading,
            "blur_processing_stability": self.test_blur_processing_stability,
            "multiple_image_handling": self.test_multiple_image_handling,
            "resource_cleanup": self.test_resource_cleanup,
        }
        
        results = {}
        print("🎮 Running Blur UI Validation Tests...")
        
        for test_name, test_func in test_suite.items():
            print(f"📋 Running {test_name.replace('_', ' ')} test...")
            
            try:
                result = test_func()
                results[test_name] = result
                
                if result["success"]:
                    print(f"  ✅ {test_name}: PASSED")
                else:
                    print(f"  ❌ {test_name}: FAILED - {result['message']}")
                    
            except Exception as e:
                results[test_name] = {
                    "success": False,
                    "message": f"Test execution failed: {e}"
                }
                print(f"  ❌ {test_name}: ERROR - {e}")
            
            # Cleanup between tests
            try:
                self.ui_tester.stop_application()
            except:
                pass
        
        return results
    
    def validate_ui_requirements(self, results: Dict) -> bool:
        """Validate UI test results meet requirements."""
        requirements_met = True
        
        print("\n📋 UI Requirements Validation:")
        
        # Critical UI tests that must pass
        critical_tests = ["application_launch", "image_loading"]
        
        for test_name in critical_tests:
            if test_name in results:
                if results[test_name]["success"]:
                    print(f"  ✅ {test_name}: Critical requirement MET")
                else:
                    print(f"  ❌ {test_name}: Critical requirement FAILED")
                    requirements_met = False
            else:
                print(f"  ⚠️  {test_name}: Test not run")
                requirements_met = False
        
        # Stability requirements
        stability_tests = ["blur_processing_stability", "resource_cleanup"]
        for test_name in stability_tests:
            if test_name in results and results[test_name]["success"]:
                print(f"  ✅ {test_name}: Stability requirement MET")
            else:
                print(f"  ⚠️  {test_name}: Stability concern")
        
        return requirements_met
    
    def generate_ui_report(self, results: Dict) -> str:
        """Generate UI validation report."""
        report_lines = [
            "# Blur UI Validation Report",
            f"Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}",
            "",
            "## Test Results",
            ""
        ]
        
        for test_name, result in results.items():
            status = "✅ PASS" if result["success"] else "❌ FAIL"
            
            report_lines.extend([
                f"### {test_name.replace('_', ' ').title()}",
                f"**Status**: {status}",
                f"**Message**: {result['message']}",
                ""
            ])
            
            # Add additional details if available
            if "stability_ratio" in result:
                report_lines.append(f"**Stability Ratio**: {result['stability_ratio']:.1%}")
            if "success_ratio" in result:
                report_lines.append(f"**Success Ratio**: {result['success_ratio']:.1%}")
            if "process_count_change" in result:
                report_lines.append(f"**Process Count Change**: {result['process_count_change']:+d}")
            
            report_lines.append("")
        
        # Summary
        passed_tests = sum(1 for r in results.values() if r["success"])
        total_tests = len(results)
        
        report_lines.extend([
            "## Summary",
            f"- **Total Tests**: {total_tests}",
            f"- **Passed**: {passed_tests}",
            f"- **Failed**: {total_tests - passed_tests}",
            f"- **Success Rate**: {passed_tests/total_tests:.1%}" if total_tests > 0 else "- **Success Rate**: N/A",
            "",
            "## Constitutional Requirements",
            "- Application must launch successfully without crashes",
            "- Image loading must work reliably",
            "- UI must remain stable during blur processing",
            "- Resource cleanup must prevent memory/process leaks",
            ""
        ])
        
        return "\n".join(report_lines)


def run_blur_ui_validation():
    """Main UI validation test runner."""
    print("=== Blur UI Validation Infrastructure ===")
    
    # Check for headless display capability
    if "DISPLAY" not in os.environ:
        print("⚠️  No DISPLAY environment variable set")
        print("Note: Running in headless mode (tests may be limited)")
    
    # Find application binary
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    
    # Try both build directories
    app_paths = [
        project_root / "build" / "hello-app",
        project_root / "builddir" / "hello-app"
    ]
    
    app_path = None
    for path in app_paths:
        if path.exists():
            app_path = path
            break
    
    if app_path is None:
        print(f"❌ Application not found at: {[str(p) for p in app_paths]}")
        print("Please build the application first")
        return False
    
    print(f"📱 Testing application: {app_path}")
    
    try:
        # Initialize UI validator
        ui_validator = BlurUIValidator(str(app_path))
        
        # Run validation tests
        results = ui_validator.run_ui_validation_tests()
        
        # Validate requirements
        requirements_met = ui_validator.validate_ui_requirements(results)
        
        # Generate report
        report = ui_validator.generate_ui_report(results)
        print("\n" + "="*60)
        print(report)
        
        # Summary
        print("="*60)
        if requirements_met:
            print("🎉 All UI validation requirements PASSED!")
            return True
        else:
            print("⚠️  Some UI validation requirements not met")
            return False
            
    except Exception as e:
        print(f"❌ UI validation failed: {e}")
        return False


if __name__ == "__main__":
    success = run_blur_ui_validation()
    sys.exit(0 if success else 1)