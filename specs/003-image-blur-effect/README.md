# Image Blur Effect with Intensity Control

## Feature Overview

Add a blur effect slider control to the HelloImageViewer that allows users to apply and adjust the intensity of a Gaussian blur effect to displayed images in real-time.

## User Story

**As a** user viewing images in the HelloImageViewer  
**I want** to apply a blur effect with adjustable intensity  
**So that** I can create artistic effects, reduce image sharpness for presentation purposes, or simulate depth-of-field effects

## Core Requirements

### Primary Functionality
1. **Blur Slider Control**
   - Add a horizontal slider (GtkScale) to the image viewer interface
   - Range: 0.0 (no blur) to 10.0 (maximum blur)  
   - Real-time preview as user drags the slider
   - Default value: 0.0 (no blur applied)

2. **Gaussian Blur Algorithm**
   - Implement Gaussian blur convolution filter
   - Support variable blur radius (0-50 pixels)
   - Maintain image quality and color accuracy
   - Handle edge conditions appropriately

3. **Performance Requirements**
   - Blur processing must complete within 500ms for HD images
   - Use progressive rendering for smooth slider interaction
   - Cache blurred versions to avoid recomputation
   - Memory usage should not exceed 200MB additional overhead

### User Interface
1. **Slider Placement**
   - Position below the B&W conversion button in header bar
   - Include blur icon and intensity label
   - Slider width: 150px minimum
   - Show current blur value (0.0-10.0) next to slider

2. **Visual Feedback**
   - Slider thumb shows current position
   - Real-time image updates while dragging
   - Loading indicator during processing (if >100ms)
   - Blur effect resets when new image is loaded

3. **Interaction Behavior**
   - Slider updates image immediately on value change
   - Works independently of B&W conversion
   - Can combine blur + B&W effects together
   - Undo/reset blur by setting slider to 0

## Technical Requirements

### Image Processing
1. **Blur Implementation**
   - Algorithm: Gaussian convolution kernel
   - Kernel size: derived from blur intensity (radius = intensity * 5)
   - Support for RGB and RGBA pixel formats
   - Maintain original image aspect ratio

2. **Performance Optimizations**
   - Use separable Gaussian filter (horizontal + vertical passes)
   - Implement multi-threading for large images
   - Cache intermediate results for common blur values
   - Progressive quality rendering during interaction

3. **Memory Management**
   - Efficient pixbuf management for blur operations
   - Cleanup temporary blur images on slider reset
   - Handle memory allocation failures gracefully
   - Prevent memory leaks during rapid slider changes

### Integration Requirements
1. **Existing Feature Compatibility**
   - Blur works with existing B&W conversion toggle
   - Both effects can be applied simultaneously  
   - Slider state persists during B&W toggle operations
   - Reset behavior affects both blur and B&W states

2. **UI Framework Integration**
   - Use GTK4 GtkScale widget for slider
   - Integrate with existing GtkHeaderBar layout
   - Maintain consistent styling with B&W button
   - Support keyboard navigation and accessibility

## Quality Attributes

### Performance
- **Target**: Blur processing <500ms for 1920x1080 images
- **Stretch Goal**: Real-time preview <100ms for responsive interaction
- **Memory**: Additional overhead <200MB for blur operations

### Usability
- **Intuitive Controls**: Slider behavior matches user expectations
- **Visual Feedback**: Clear indication of blur intensity and processing state
- **Error Handling**: Graceful handling of processing failures or memory issues

### Compatibility  
- **Platform Support**: Works on macOS and Linux (Ubuntu)
- **Image Formats**: All formats supported by GdkPixbuf (PNG, JPEG, TIFF, etc.)
- **Build Systems**: Compatible with both Meson and CMake builds

## Success Criteria

### Functional Success
✅ **Blur Slider**: Horizontal slider with 0-10 range controls blur intensity  
✅ **Real-time Preview**: Image updates as user drags slider  
✅ **Gaussian Algorithm**: High-quality blur using separable Gaussian filter  
✅ **Performance Target**: HD image processing <500ms  
✅ **Effect Combination**: Blur works with existing B&W conversion  

### Quality Success
✅ **Smooth Interaction**: No visible lag during slider manipulation  
✅ **Memory Efficiency**: No memory leaks, reasonable memory usage  
✅ **Error Robustness**: Handles edge cases and failures gracefully  
✅ **UI Integration**: Consistent styling and layout with existing controls  
✅ **Accessibility**: Keyboard navigation and screen reader support  

## Out of Scope

### Current Release Exclusions
- Advanced filter effects (sharpen, emboss, etc.)
- Multiple blur algorithms (motion blur, radial blur)  
- Blur masking or selective area blur
- Preset blur intensity values or favorites
- Blur effect animation or transitions
- Integration with external image editing tools
- Batch processing of multiple images
- Export of blurred images to file

### Future Considerations
- Additional image filters and effects
- Filter combination and layering system
- Custom blur kernel shapes
- Performance optimizations using GPU acceleration

## Risk Assessment

### Technical Risks
- **Performance**: Large images may cause UI blocking during blur processing
- **Memory**: Multiple cached blur versions could exhaust system memory  
- **Quality**: Poor blur algorithm implementation could produce artifacts

### Mitigation Strategies
- Implement progressive rendering and background processing
- Use LRU cache with configurable memory limits for blur versions
- Test with various image sizes and validate blur quality

### User Experience Risks  
- **Complexity**: Adding too many controls could overwhelm users
- **Consistency**: Blur slider behavior might conflict with B&W toggle expectations

### Mitigation Approaches
- Keep slider design minimal and intuitive
- Maintain consistent interaction patterns with existing controls
- Comprehensive user testing with various image types and sizes