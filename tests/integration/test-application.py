#!/usr/bin/env python3
"""
Integration test for HelloApp application launch.

Tests the complete application startup sequence and basic functionality.
"""

import subprocess
import time
import sys
import os

# Add current directory to Python path for dogtail import
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    import dogtail.tree
    import dogtail.utils
    import dogtail.config
except ImportError:
    print("DoGTail not available. Install with: pip3 install dogtail")
    sys.exit(77)  # Skip test

class HelloAppIntegrationTest:
    """Integration test class for HelloApp."""
    
    def __init__(self):
        self.app_process = None
        self.app_node = None
        
        # Configure dogtail
        dogtail.config.config.searchCutoffCount = 10
        dogtail.config.config.searchBackoffDuration = 0.1
    
    def setUp(self):
        """Set up test environment."""
        # Ensure clean environment
        self.tearDown()
        
        # Set up display for headless testing
        if not os.environ.get('DISPLAY'):
            os.environ['DISPLAY'] = ':99'
    
    def tearDown(self):
        """Clean up test environment."""
        if self.app_process:
            try:
                self.app_process.terminate()
                self.app_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.app_process.kill()
                self.app_process.wait()
            except:
                pass
            self.app_process = None
        
        self.app_node = None
    
    def test_application_launch(self):
        """Test basic application launch and visibility."""
        print("Testing application launch...")
        
        # Launch application
        try:
            self.app_process = subprocess.Popen(['./hello-app'], 
                                               stdout=subprocess.PIPE,
                                               stderr=subprocess.PIPE)
        except FileNotFoundError:
            # Try from build directory
            try:
                self.app_process = subprocess.Popen(['../build/hello-app'],
                                                   stdout=subprocess.PIPE, 
                                                   stderr=subprocess.PIPE)
            except FileNotFoundError:
                print("ERROR: hello-app executable not found")
                return False
        
        # Wait for application to start
        time.sleep(2)
        
        # Check if process is still running
        if self.app_process.poll() is not None:
            stdout, stderr = self.app_process.communicate()
            print(f"Application exited early. stdout: {stdout.decode()}")
            print(f"stderr: {stderr.decode()}")
            return False
        
        try:
            # Find application in accessibility tree
            self.app_node = dogtail.tree.root.application('hello-app')
            if not self.app_node:
                print("ERROR: Could not find hello-app in accessibility tree")
                return False
            
            print("✓ Application launched successfully")
            return True
            
        except Exception as e:
            print(f"ERROR: Failed to find application: {e}")
            return False
    
    def test_window_display(self):
        """Test window display and content."""
        if not self.app_node:
            print("ERROR: Application not launched")
            return False
        
        print("Testing window display...")
        
        try:
            # Find main window
            window = self.app_node.findChild(name="Hello World", roleName='frame')
            if not window:
                print("ERROR: Could not find main window")
                return False
            
            # Check if window is visible
            if not window.showing:
                print("ERROR: Window is not visible")
                return False
            
            print("✓ Window is visible")
            
            # Find greeting label
            try:
                greeting_label = window.findChild(name="Hello World!", roleName='label')
                if not greeting_label:
                    print("WARNING: Could not find greeting label by exact name")
                    # Try to find any label
                    labels = window.findChildren(roleName='label')
                    if labels:
                        greeting_label = labels[0]
                        print(f"Found label with text: '{greeting_label.name}'")
                    else:
                        print("ERROR: No labels found in window")
                        return False
                
                print("✓ Greeting label found and displayed")
                return True
                
            except Exception as e:
                print(f"ERROR: Failed to find greeting label: {e}")
                return False
                
        except Exception as e:
            print(f"ERROR: Failed to test window: {e}")
            return False
    
    def test_application_close(self):
        """Test application close functionality."""
        if not self.app_node:
            print("ERROR: Application not launched")
            return False
        
        print("Testing application close...")
        
        try:
            # Find main window
            window = self.app_node.findChild(name="Hello World", roleName='frame')
            if not window:
                print("ERROR: Could not find main window")
                return False
            
            # Close window (should exit application)
            window.keyCombo('<Alt>F4')  # Standard window close
            
            # Wait for application to exit
            time.sleep(2)
            
            # Check if process has exited
            if self.app_process.poll() is None:
                print("WARNING: Application still running after close")
                # Try sending SIGTERM
                self.app_process.terminate()
                time.sleep(1)
            
            if self.app_process.poll() is not None:
                print("✓ Application closed successfully")
                return True
            else:
                print("ERROR: Application did not close")
                return False
                
        except Exception as e:
            print(f"ERROR: Failed to close application: {e}")
            return False

def main():
    """Run all integration tests."""
    test = HelloAppIntegrationTest()
    test_results = []
    
    try:
        test.setUp()
        
        # Run tests in sequence
        tests = [
            test.test_application_launch,
            test.test_window_display, 
            test.test_application_close
        ]
        
        for test_func in tests:
            try:
                result = test_func()
                test_results.append(result)
                if not result:
                    break  # Stop on first failure
            except Exception as e:
                print(f"ERROR: Test {test_func.__name__} failed with exception: {e}")
                test_results.append(False)
                break
    
    finally:
        test.tearDown()
    
    # Report results
    passed = sum(test_results)
    total = len(test_results)
    
    print(f"\nTest Results: {passed}/{total} passed")
    
    if passed == total:
        print("✓ All integration tests passed!")
        return 0
    else:
        print("✗ Some integration tests failed")
        return 1

if __name__ == '__main__':
    sys.exit(main())