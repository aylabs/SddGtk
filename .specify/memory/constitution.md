<!--
SYNC IMPACT REPORT - Constitutional Amendment v1.0.1

Version Change: 1.0.0 → 1.0.1
Amendment Date: 2026-02-03

Modified Principles:
- V. Testing Requirements: Added Ubuntu-only pipeline testing requirement

Added Sections:
- Pipeline Testing specification under Testing Requirements

Templates Status:
✅ Updated: Constitution aligns with existing templates
⚠ Review: CI/CD configurations should verify Ubuntu platform requirement

Follow-up Actions:
- Verify all pipeline configurations use Ubuntu runners exclusively  
- Update CI/CD documentation to reflect Ubuntu-only testing policy
- Consider adding pipeline validation checks for platform compliance
-->

# GTK Cross-Platform GUI Constitution

## Core Principles

### I. GTK-First GUI Development (NON-NEGOTIABLE)
All graphical user interface applications MUST be developed using GTK (GIMP Toolkit) as the primary and exclusive GUI framework. This applies to all target platforms:
- **Linux**: Native GTK support through system package managers
- **Windows**: GTK applications via MSYS2/MinGW-w64 or official GTK bundles
- **macOS**: GTK applications via Homebrew, MacPorts, or official GTK bundles

No exceptions are permitted for native platform UI frameworks (Win32, Cocoa, Qt, etc.). All GUI components must utilize GTK widgets and follow GTK design patterns.

### II. Cross-Platform Compatibility
GUI applications MUST function identically across Linux, Windows, and macOS environments:
- Use GTK's cross-platform abstractions for file operations, threading, and system integration
- Test on all three target platforms before release
- Document platform-specific installation and deployment procedures
- Ensure consistent visual appearance and behavior across platforms

### III. GTK Best Practices
All GTK development MUST adhere to established best practices:
- Use GtkBuilder for UI definition with .ui XML files
- Implement proper signal handling and event management
- Follow GTK naming conventions and coding standards
- Utilize GTK's built-in accessibility features
- Implement proper memory management with GObject reference counting

### IV. Library-First Architecture
GUI functionality MUST be organized as reusable libraries:
- Separate business logic from GUI presentation layers
- Create modular GTK widget libraries for common UI patterns
- Ensure GUI libraries are independently testable
- Document public APIs for all GUI components
- Provide both programmatic and GtkBuilder interfaces

### V. Testing Requirements
All GTK GUI code MUST be thoroughly tested:
- Unit tests for business logic separated from GUI
- GTK widget testing using appropriate testing frameworks
- Visual regression testing for UI consistency
- Automated testing across all target platforms
- Manual testing procedures for platform-specific behaviors
- **Pipeline Testing**: All automated CI/CD pipeline testing MUST be executed exclusively on Ubuntu platform to ensure consistent and reliable build environments

## Platform Integration Standards

### Development Environment Setup
- Linux: Use system GTK development packages (libgtk-3-dev or libgtk-4-dev)
- Windows: Use MSYS2 environment with mingw-w64-gtk packages
- macOS: Use Homebrew gtk+3 or gtk4 packages
- All platforms must support the same GTK major version

### Deployment and Distribution
- Provide native installation packages for each platform
- Bundle GTK runtime dependencies when system packages unavailable
- Document GTK version requirements and compatibility matrix
- Ensure consistent GTK theming across platforms

### Performance Standards
- GUI responsiveness: Maximum 100ms response time for user interactions
- Memory efficiency: Proper GObject cleanup and reference management
- Startup time: Applications must launch within 3 seconds on target hardware

## Development Workflow

### Code Review Requirements
- All GUI code changes require review for GTK compliance
- Cross-platform testing verification before merge approval
- Accessibility compliance verification for all new UI components
- Performance impact assessment for GUI modifications

### Quality Gates
- Successful compilation and testing on all three target platforms
- GTK coding standards compliance verification
- Memory leak detection using Valgrind (Linux) and similar tools
- Visual consistency verification across platforms

## Governance

This constitution supersedes all other development practices and framework choices for GUI development. The GTK-First principle is non-negotiable and applies to all current and future GUI applications.

Amendments to this constitution require:
1. Technical justification for any proposed changes
2. Approval from project maintainers
3. Migration plan for existing non-compliant code
4. Updated testing and deployment procedures

All pull requests and code reviews must verify compliance with these constitutional requirements. Deviations from GTK-First development must be explicitly justified and approved through the constitutional amendment process.

**Version**: 1.0.1 | **Ratified**: 2026-02-01 | **Last Amended**: 2026-02-03
