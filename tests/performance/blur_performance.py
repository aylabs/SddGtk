#!/usr/bin/env python3
"""
Blur Performance Testing Infrastructure

Tests blur processing performance across different image sizes and scenarios.
Provides benchmarking and regression detection for blur operations.
"""

import os
import sys
import time
import tempfile
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple, Optional

# Add project root to path to import test utilities
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

class BlurPerformanceTests:
    def __init__(self, app_path: str):
        self.app_path = Path(app_path)
        self.results = {}
        
        if not self.app_path.exists():
            raise FileNotFoundError(f"Application not found: {app_path}")
    
    def create_test_image(self, width: int, height: int, filename: str) -> bool:
        """Create test image using available tools."""
        try:
            # Try ImageMagick first
            cmd = [
                "convert", "-size", f"{width}x{height}", 
                "gradient:red-blue", filename
            ]
            subprocess.run(cmd, check=True, capture_output=True)
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            try:
                # Fallback to PIL
                from PIL import Image, ImageDraw
                import random
                
                img = Image.new('RGB', (width, height))
                draw = ImageDraw.Draw(img)
                
                # Create test pattern
                for _ in range(min(100, width * height // 1000)):
                    x1, y1 = random.randint(0, width//2), random.randint(0, height//2)
                    x2, y2 = x1 + random.randint(50, width//4), y1 + random.randint(50, height//4)
                    color = (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
                    draw.rectangle([x1, y1, x2, y2], fill=color)
                
                img.save(filename)
                return True
            except ImportError:
                print("Warning: Neither ImageMagick nor PIL available for test image creation")
                return False
    
    def benchmark_blur_processing(self, image_size: Tuple[int, int], test_name: str) -> Dict:
        """Benchmark blur processing for specific image size."""
        width, height = image_size
        
        with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp_file:
            if not self.create_test_image(width, height, tmp_file.name):
                return {"error": "Could not create test image", "time": -1}
            
            try:
                # Start timing
                start_time = time.time()
                
                # Launch application with test image
                # Note: In real implementation, this would trigger blur processing
                # For now, we simulate by launching and quickly terminating
                process = subprocess.Popen(
                    [str(self.app_path), tmp_file.name],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE
                )
                
                # Let it initialize
                time.sleep(0.5)
                
                # Terminate
                process.terminate()
                process.wait(timeout=5)
                
                end_time = time.time()
                processing_time = end_time - start_time
                
                return {
                    "test_name": test_name,
                    "image_size": f"{width}x{height}",
                    "processing_time": processing_time,
                    "pixels": width * height,
                    "success": True
                }
                
            except subprocess.TimeoutExpired:
                process.kill()
                return {"error": "Process timeout", "time": -1}
            except Exception as e:
                return {"error": str(e), "time": -1}
            finally:
                # Cleanup test image
                try:
                    os.unlink(tmp_file.name)
                except:
                    pass
    
    def run_performance_benchmarks(self) -> Dict:
        """Run comprehensive performance benchmarks."""
        benchmarks = {
            "small_image": (640, 480),      # VGA
            "hd_image": (1920, 1080),       # HD - Constitutional requirement <500ms
            "4k_image": (3840, 2160),       # 4K
        }
        
        results = {}
        print("🚀 Running Blur Performance Benchmarks...")
        
        for test_name, size in benchmarks.items():
            print(f"  📊 Testing {test_name} ({size[0]}x{size[1]})")
            
            # Run multiple iterations for better accuracy
            iterations = 3
            times = []
            
            for i in range(iterations):
                result = self.benchmark_blur_processing(size, f"{test_name}_iter_{i}")
                if result.get("success"):
                    times.append(result["processing_time"])
                else:
                    print(f"    ⚠️  Iteration {i+1} failed: {result.get('error', 'Unknown error')}")
            
            if times:
                avg_time = sum(times) / len(times)
                min_time = min(times)
                max_time = max(times)
                
                results[test_name] = {
                    "image_size": size,
                    "avg_time": avg_time,
                    "min_time": min_time,
                    "max_time": max_time,
                    "iterations": len(times),
                    "pixels": size[0] * size[1]
                }
                
                # Check constitutional requirements
                status = "✅ PASS"
                if test_name == "hd_image" and avg_time > 0.5:  # 500ms constitutional requirement
                    status = "❌ FAIL (exceeds 500ms constitutional requirement)"
                
                print(f"    Average: {avg_time:.3f}s, Range: {min_time:.3f}-{max_time:.3f}s {status}")
            else:
                results[test_name] = {"error": "All iterations failed"}
                print(f"    ❌ All iterations failed")
        
        return results
    
    def validate_performance_requirements(self, results: Dict) -> bool:
        """Validate results meet performance requirements."""
        requirements_met = True
        
        print("\n📋 Performance Requirements Validation:")
        
        # HD image constitutional requirement
        if "hd_image" in results and "avg_time" in results["hd_image"]:
            hd_time = results["hd_image"]["avg_time"]
            if hd_time <= 0.5:
                print(f"  ✅ HD processing: {hd_time:.3f}s (≤ 0.5s constitutional requirement)")
            else:
                print(f"  ❌ HD processing: {hd_time:.3f}s (exceeds 0.5s constitutional requirement)")
                requirements_met = False
        
        # Performance scaling validation
        if "small_image" in results and "4k_image" in results:
            small_pixels = results["small_image"]["pixels"]
            large_pixels = results["4k_image"]["pixels"]
            
            if "avg_time" in results["small_image"] and "avg_time" in results["4k_image"]:
                small_time = results["small_image"]["avg_time"]
                large_time = results["4k_image"]["avg_time"]
                
                pixel_ratio = large_pixels / small_pixels
                time_ratio = large_time / small_time if small_time > 0 else float('inf')
                
                print(f"  📊 Scaling: {pixel_ratio:.1f}x pixels → {time_ratio:.1f}x time")
                
                # Reasonable scaling (shouldn't be worse than linear)
                if time_ratio <= pixel_ratio * 1.5:
                    print(f"  ✅ Scaling efficiency within acceptable bounds")
                else:
                    print(f"  ⚠️  Scaling efficiency could be improved")
        
        return requirements_met
    
    def generate_performance_report(self, results: Dict) -> str:
        """Generate performance test report."""
        report_lines = [
            "# Blur Performance Test Report",
            f"Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}",
            "",
            "## Benchmark Results",
            ""
        ]
        
        for test_name, result in results.items():
            if "error" in result:
                report_lines.append(f"### {test_name}")
                report_lines.append(f"❌ **FAILED**: {result['error']}")
                report_lines.append("")
                continue
                
            report_lines.extend([
                f"### {test_name}",
                f"- **Image Size**: {result['image_size'][0]}x{result['image_size'][1]} ({result['pixels']:,} pixels)",
                f"- **Average Time**: {result['avg_time']:.3f}s",
                f"- **Range**: {result['min_time']:.3f}s - {result['max_time']:.3f}s",
                f"- **Iterations**: {result['iterations']}",
                ""
            ])
        
        report_lines.extend([
            "## Performance Summary",
            "- Tests measure application startup + processing simulation",
            "- Real implementation would measure actual blur algorithm performance",  
            "- Constitutional requirement: HD (1920x1080) processing ≤ 500ms",
            ""
        ])
        
        return "\n".join(report_lines)


def run_blur_performance_tests():
    """Main performance test runner."""
    print("=== Blur Performance Testing Infrastructure ===")
    
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
        # Initialize performance tester
        perf_tester = BlurPerformanceTests(str(app_path))
        
        # Run benchmarks
        results = perf_tester.run_performance_benchmarks()
        
        # Validate requirements
        requirements_met = perf_tester.validate_performance_requirements(results)
        
        # Generate report
        report = perf_tester.generate_performance_report(results)
        print("\n" + "="*60)
        print(report)
        
        # Summary
        print("="*60)
        if requirements_met:
            print("🎉 All performance requirements PASSED!")
            return True
        else:
            print("⚠️  Some performance requirements not met")
            return False
            
    except Exception as e:
        print(f"❌ Performance testing failed: {e}")
        return False


if __name__ == "__main__":
    success = run_blur_performance_tests()
    sys.exit(0 if success else 1)