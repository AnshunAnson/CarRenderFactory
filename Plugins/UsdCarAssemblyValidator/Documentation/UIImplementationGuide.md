# USD Car Assembly Validator - UI Implementation Guide

## Overview

The USD Car Assembly Validator plugin provides a comprehensive editor UI for validating car assemblies in USD files and injecting them into Unreal Engine CCR (Configurable Car Runtime) blueprints.

## UI Layout Structure

### 1. Top Section - File Selection & Control Buttons
```
┌─────────────────────────────────────────────────────────────┐
│  Select USD: [Choose USD File...]                          │
│  [Reload Stage] [Scan Variants] [Verify Assembly]          │
└─────────────────────────────────────────────────────────────┘
```

**Components:**
- **USD File Dropdown**: Select a USD stage file to load
- **Reload Stage Button**: Reload the currently selected USD stage
- **Scan Variants Button**: Scan and load available variants from the stage
- **Verify Assembly Button**: Validate the assembly structure

### 2. Middle Section - Three-Panel Content Area
```
┌──────────────┬──────────────────┬─────────────────────────┐
│ Variant Sets │ Current Variant  │ Identified Parts        │
│              │                  │                         │
│ • Variant 1  │  None Selected   │ • Body                  │
│ • Variant 2  │  (Display in     │ • Wheel_FL              │
│ • Variant 3  │   Yellow Text)   │ • Wheel_FR              │
│              │                  │ • Wheel_RL              │
│              │                  │ • Wheel_RR              │
│              │                  │ • Caliper_FL            │
│              │                  │ • SteeringWheel         │
└──────────────┴──────────────────┴─────────────────────────┘
```

**Left Panel - Variant Sets (25% width):**
- TreeView displaying all available variant sets
- User can select variants to inspect

**Center Panel - Current Variant (25% width):**
- Text display showing the currently selected variant
- Highlighted in yellow for visibility
- Shows "None Selected" when no variant is chosen

**Right Panel - Identified Parts (50% width):**
- ListView of all identified assembly parts
- Default parts shown:
  - Body
  - Wheel_FL, Wheel_FR, Wheel_RL, Wheel_RR
  - Caliper_FL
  - SteeringWheel
- Can be dynamically updated based on USD content

### 3. Bottom Section - Actions & Logging
```
┌─────────────────────────────────────────────────────────────┐
│          [Inject to Selected CCR Blueprint]                │
│  Success: 0     Warning: 0     Failed: 0                   │
├─────────────────────────────────────────────────────────────┤
│ [23:45:12] USD stage loaded successfully.                  │
│ [23:45:13] Scanning for variants...                        │
│ [23:45:14] Found 3 variant sets.                           │
│ [23:45:15] Assembly verification completed successfully.   │
└─────────────────────────────────────────────────────────────┘
```

**Components:**
- **Inject Button**: Primary action to inject assembly into CCR blueprint
- **Statistics**: Real-time counters for Success/Warning/Failed operations
- **Log Output**: Multi-line read-only text box with:
  - Timestamp for each message
  - Color coding: Green (success), Yellow (warning), Red (error)
  - Scrollable history

## Features

### 1. USD File Management
- Load USD stage files
- Support for multiple variant sets
- Reload functionality without restarting

### 2. Assembly Validation
- Verify assembly structure and hierarchy
- Scan for available variants
- Identify and list assembly parts
- Generate validation reports

### 3. Data Injection
- Inject validated assemblies into CCR blueprints
- Support for multiple CCR targets
- Injection status feedback

### 4. Logging & Monitoring
- Comprehensive operation logging
- Timestamp for each operation
- Success/Warning/Error statistics
- Real-time feedback

## Data Structures

### FUsdStageData
```cpp
struct FUsdStageData
{
    FString StageFilePath;           // Path to USD stage file
    TArray<FString> VariantSets;     // List of available variants
    TArray<FString> IdentifiedParts; // List of assembly parts
    FString CurrentVariant;          // Currently selected variant
};
```

### FAssemblyValidationResult
```cpp
struct FAssemblyValidationResult
{
    bool bIsValid;                      // Is assembly valid
    int32 WarningCount;                 // Number of warnings
    int32 ErrorCount;                   // Number of errors
    TArray<FString> ValidationMessages; // Detailed messages
};
```

## Usage Flow

1. **Load USD File**
   - User clicks on USD file dropdown
   - Browser opens to select USD file
   - File is loaded and stage parsed

2. **Scan Variants**
   - User clicks "Scan Variants" button
   - Available variants are retrieved and displayed
   - List updated in left panel

3. **Select Variant**
   - User clicks on a variant in left panel
   - Selected variant displayed in center panel

4. **Verify Assembly**
   - User clicks "Verify Assembly" button
   - Assembly structure is validated
   - Results logged with success/warning/error counts

5. **Inject to CCR**
   - User selects target CCR blueprint
   - Clicks "Inject to Selected CCR Blueprint"
   - Assembly data injected into blueprint
   - Confirmation logged

## Implementation Notes

### UI Framework
- Built with Slate (Unreal's declarative UI framework)
- Uses SCompoundWidget as base class
- Responsive layout with splitters

### Data Handling
- Manages USD stage data internally
- Maintains variant and part lists
- Validates assembly structure

### Logging System
- Timestamped log entries
- Color-coded messages (Success/Warning/Error)
- Persistent log display with scrolling

### Error Handling
- Validation error reporting
- User-friendly error messages
- Graceful degradation for missing data

## Extension Points

### Adding New Validators
1. Create new validation function in AssemblyValidatorUIManager
2. Call from VerifyAssembly method
3. Log results using AddLogMessage

### Adding New Assembly Parts
1. Update IdentifiedParts array initialization
2. Query USD stage for actual parts
3. Update ListView dynamically

### Custom CCR Blueprint Integration
1. Implement CCR-specific injection logic
2. Handle blueprint modification safely
3. Validate after injection

## Known Limitations

1. USD file loading - requires USD plugin support
2. CCR blueprint injection - requires CCR framework setup
3. Real-time variant preview - planned for future

## Future Enhancements

- 3D viewport preview of assembly
- Real-time variant switching
- Assembly comparison tools
- Batch processing support
- Custom validation rule editor
- Assembly template library
