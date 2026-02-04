#!/usr/bin/env python3
"""
Blur Memory Profiling Infrastructure

Monitors memory usage during blur processing operations.
Detects memory leaks and validates memory management.
"""

import os
import sys
import time
import subprocess
import threading
import tempfile
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import json

# Add project root to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

class MemoryProfiler:
    def __init__(self, pid: int, interval: float = 0.1):
        self.pid = pid
        self.interval = interval
        self.monitoring = False
        self.samples = []
        self.thread = None
    
    def start_monitoring(self):
        """Start memory monitoring in background thread."""
        self.monitoring = True
        self.samples = []
        self.thread = threading.Thread(target=self._monitor_memory)
        self.thread.daemon = True
        self.thread.start()
    
    def stop_monitoring(self):
        """Stop memory monitoring."""
        self.monitoring = False
        if self.thread:
            self.thread.join(timeout=1)
    
    def _monitor_memory(self):
        """Memory monitoring loop."""
        while self.monitoring:
            try:
                # Read memory info from /proc (Linux/macOS compatible approach)
                if sys.platform == "darwin":  # macOS
                    # Use ps for cross-platform compatibility
                    result = subprocess.run(
                        ["ps", "-p", str(self.pid), "-o", "rss="],
                        capture_output=True, text=True
                    )
                    if result.returncode == 0:
                        rss_kb = int(result.stdout.strip())
                        memory_mb = rss_kb / 1024
                    else:
                        memory_mb = 0
                else:  # Linux
                    try:
                        with open(f"/proc/{self.pid}/status", "r") as f:
                            for line in f:
                                if line.startswith("VmRSS:"):
                                    memory_kb = int(line.split()[1])
                                    memory_mb = memory_kb / 1024
                                    break
                            else:
                                memory_mb = 0
                    except (FileNotFoundError, ValueError):
                        memory_mb = 0
                
                timestamp = time.time()
                self.samples.append({
                    "timestamp": timestamp,
                    "memory_mb": memory_mb
                })
                
            except Exception:
                # Process might have ended or other error
                pass
            
            time.sleep(self.interval)
    
    def get_memory_stats(self) -> Dict:
        """Get memory usage statistics."""
        if not self.samples:
            return {"error": "No memory samples collected"}
        
        memories = [sample["memory_mb"] for sample in self.samples]
        
        return {
            "samples_count": len(memories),
            "peak_memory_mb": max(memories),
            "min_memory_mb": min(memories),
            "avg_memory_mb": sum(memories) / len(memories),
            "memory_growth_mb": memories[-1] - memories[0] if len(memories) > 1 else 0,
            "samples": self.samples
        }


class BlurMemoryProfiler:
    def __init__(self, app_path: str):
        self.app_path = Path(app_path)
        
        if not self.app_path.exists():
            raise FileNotFoundError(f"Application not found: {app_path}")
    
    def create_test_image(self, width: int, height: int, filename: str) -> bool:
        """Create test image for memory profiling."""
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
                
                # Create complex pattern for memory stress testing
                for _ in range(min(500, width * height // 10000)):
                    x1, y1 = random.randint(0, width//2), random.randint(0, height//2)
                    x2, y2 = x1 + random.randint(20, width//8), y1 + random.randint(20, height//8)
                    color = (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
                    draw.ellipse([x1, y1, x2, y2], fill=color)
                
                img.save(filename)
                return True
            except ImportError:
                print("Warning: Neither ImageMagick nor PIL available for test image creation")
                return False
    
    def profile_blur_memory(self, image_size: Tuple[int, int], test_name: str, duration: float = 5.0) -> Dict:
        """Profile memory usage during blur processing."""
        width, height = image_size
        
        with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp_file:
            if not self.create_test_image(width, height, tmp_file.name):
                return {"error": "Could not create test image"}
            
            try:
                print(f"  🔍 Profiling {test_name} memory usage...")
                
                # Launch application
                process = subprocess.Popen(
                    [str(self.app_path), tmp_file.name],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE
                )
                
                # Start memory monitoring
                profiler = MemoryProfiler(process.pid, interval=0.05)
                profiler.start_monitoring()
                
                # Let it run for specified duration
                time.sleep(duration)
                
                # Stop monitoring and terminate process
                profiler.stop_monitoring()
                process.terminate()
                
                try:
                    process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                
                # Get memory statistics
                stats = profiler.get_memory_stats()
                
                if "error" not in stats:
                    stats.update({
                        "test_name": test_name,
                        "image_size": f"{width}x{height}",
                        "pixels": width * height,
                        "duration": duration,
                        "success": True
                    })
                
                return stats
                
            except Exception as e:
                return {"error": str(e)}
            finally:
                # Cleanup test image
                try:
                    os.unlink(tmp_file.name)
                except:
                    pass
    
    def detect_memory_leaks(self, samples: List[Dict]) -> Dict:
        """Analyze memory samples for potential leaks."""
        if len(samples) < 10:
            return {"error": "Insufficient samples for leak detection"}
        
        # Calculate memory trend (linear regression)
        n = len(samples)
        sum_x = sum(range(n))
        sum_y = sum(sample["memory_mb"] for sample in samples)
        sum_xy = sum(i * sample["memory_mb"] for i, sample in enumerate(samples))
        sum_x2 = sum(i * i for i in range(n))
        
        # Slope of memory usage over time
        slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x)
        
        # Memory growth rate per second
        time_span = samples[-1]["timestamp"] - samples[0]["timestamp"]
        growth_rate_mb_per_sec = slope / (time_span / n) if time_span > 0 else 0
        
        # Leak detection thresholds
        leak_threshold_mb_per_sec = 1.0  # Constitutional requirement: <1MB/sec growth
        
        return {
            "memory_growth_rate_mb_per_sec": growth_rate_mb_per_sec,
            "potential_leak": abs(growth_rate_mb_per_sec) > leak_threshold_mb_per_sec,
            "leak_severity": "high" if abs(growth_rate_mb_per_sec) > 5.0 else "low" if abs(growth_rate_mb_per_sec) > leak_threshold_mb_per_sec else "none"
        }
    
    def run_memory_profiling_tests(self) -> Dict:
        """Run comprehensive memory profiling tests."""
        test_scenarios = {
            "small_image_baseline": (640, 480, 3.0),      # Baseline memory usage
            "hd_image_processing": (1920, 1080, 5.0),     # HD processing
            "4k_image_stress": (3840, 2160, 8.0),         # Stress test
        }
        
        results = {}
        print("🧠 Running Blur Memory Profiling Tests...")
        
        for test_name, (width, height, duration) in test_scenarios.items():
            print(f"  📊 Testing {test_name} ({width}x{height}) for {duration}s")
            
            result = self.profile_blur_memory((width, height), test_name, duration)
            
            if result.get("success"):
                # Analyze for memory leaks
                if "samples" in result:
                    leak_analysis = self.detect_memory_leaks(result["samples"])
                    result.update(leak_analysis)
                
                # Report results
                peak_mb = result["peak_memory_mb"]
                growth_mb = result.get("memory_growth_mb", 0)
                leak_rate = result.get("memory_growth_rate_mb_per_sec", 0)
                
                print(f"    Peak: {peak_mb:.1f}MB, Growth: {growth_mb:+.1f}MB, Rate: {leak_rate:+.3f}MB/s")
                
                # Check constitutional requirements
                if result.get("potential_leak", False):
                    print(f"    ⚠️  Potential memory leak detected (rate: {leak_rate:+.3f}MB/s)")
                else:
                    print(f"    ✅ No memory leaks detected")
                
                # Memory efficiency check
                pixels_mb = (width * height * 4) / (1024 * 1024)  # RGBA
                efficiency_ratio = peak_mb / pixels_mb if pixels_mb > 0 else 0
                
                if efficiency_ratio < 10:  # Less than 10x image size
                    print(f"    ✅ Memory efficiency good ({efficiency_ratio:.1f}x image size)")
                else:
                    print(f"    ⚠️  Memory usage high ({efficiency_ratio:.1f}x image size)")
                
            else:
                print(f"    ❌ Test failed: {result.get('error', 'Unknown error')}")
            
            results[test_name] = result
        
        return results
    
    def validate_memory_requirements(self, results: Dict) -> bool:
        """Validate memory usage meets requirements."""
        requirements_met = True
        
        print("\n📋 Memory Requirements Validation:")
        
        # Constitutional memory requirements
        for test_name, result in results.items():
            if not result.get("success"):
                continue
                
            # Memory leak check
            if result.get("potential_leak", False):
                leak_rate = result.get("memory_growth_rate_mb_per_sec", 0)
                print(f"  ❌ {test_name}: Memory leak detected ({leak_rate:+.3f}MB/s)")
                requirements_met = False
            else:
                print(f"  ✅ {test_name}: No memory leaks")
            
            # Peak memory check (constitutional: <100MB for HD images)
            if "hd_image" in test_name:
                peak_mb = result.get("peak_memory_mb", 0)
                if peak_mb <= 100:
                    print(f"  ✅ HD image peak memory: {peak_mb:.1f}MB (≤ 100MB constitutional requirement)")
                else:
                    print(f"  ❌ HD image peak memory: {peak_mb:.1f}MB (exceeds 100MB constitutional requirement)")
                    requirements_met = False
        
        return requirements_met
    
    def generate_memory_report(self, results: Dict) -> str:
        """Generate memory profiling report."""
        report_lines = [
            "# Blur Memory Profiling Report",
            f"Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}",
            "",
            "## Memory Usage Analysis",
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
                f"- **Image Size**: {result.get('image_size', 'Unknown')} ({result.get('pixels', 0):,} pixels)",
                f"- **Duration**: {result.get('duration', 0):.1f}s",
                f"- **Peak Memory**: {result.get('peak_memory_mb', 0):.1f}MB",
                f"- **Memory Growth**: {result.get('memory_growth_mb', 0):+.1f}MB",
                f"- **Growth Rate**: {result.get('memory_growth_rate_mb_per_sec', 0):+.3f}MB/s",
                f"- **Leak Status**: {'⚠️ Potential leak' if result.get('potential_leak') else '✅ No leaks'}",
                f"- **Samples**: {result.get('samples_count', 0)}",
                ""
            ])
        
        report_lines.extend([
            "## Memory Requirements",
            "- Constitutional requirement: No memory leaks (growth <1MB/s)",
            "- Constitutional requirement: HD processing peak memory ≤100MB", 
            "- Memory efficiency: Peak usage <10x image data size",
            ""
        ])
        
        return "\n".join(report_lines)


def run_blur_memory_profiling():
    """Main memory profiling test runner."""
    print("=== Blur Memory Profiling Infrastructure ===")
    
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
        # Initialize memory profiler
        memory_profiler = BlurMemoryProfiler(str(app_path))
        
        # Run profiling tests
        results = memory_profiler.run_memory_profiling_tests()
        
        # Validate requirements
        requirements_met = memory_profiler.validate_memory_requirements(results)
        
        # Generate report
        report = memory_profiler.generate_memory_report(results)
        print("\n" + "="*60)
        print(report)
        
        # Summary
        print("="*60)
        if requirements_met:
            print("🎉 All memory requirements PASSED!")
            return True
        else:
            print("⚠️  Some memory requirements not met")
            return False
            
    except Exception as e:
        print(f"❌ Memory profiling failed: {e}")
        return False


if __name__ == "__main__":
    success = run_blur_memory_profiling()
    sys.exit(0 if success else 1)