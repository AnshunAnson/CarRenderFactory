# USD Car Assembly Validator Plugin

## Quick Start

### Opening the Validator Window

1. In Unreal Editor, go to **Tools** menu
2. Look for **USD Car Assembly Validator** option
3. Click to open the validator panel

### Basic Workflow

#### Step 1: Load a USD File
1. Click the **"Choose USD File..."** dropdown button
2. Select your USD stage file containing car assembly data
3. The stage will be parsed and loaded

#### Step 2: Scan Variants
1. Click the **"Scan Variants"** button
2. Available variants from the USD file will appear in the left panel
3. Variants can include color variations, wheel types, interior options, etc.

#### Step 3: Select and Inspect Variant
1. Click on a variant in the **"Variant Sets"** left panel
2. The currently selected variant will appear in the center panel (yellow text)
3. The **"Identified Parts"** panel on the right shows assembly parts:
   - Body
   - Wheel_FL (Front Left)
   - Wheel_FR (Front Right)
   - Wheel_RL (Rear Left)
   - Wheel_RR (Rear Right)
   - Caliper_FL (Front Left Brake Caliper)
   - SteeringWheel

#### Step 4: Verify Assembly
1. Click the **"Verify Assembly"** button
2. The plugin will validate:
   - Assembly hierarchy
   - Part naming conventions
   - Component relationships
   - Required attributes
3. Results appear in the log with:
   - **Success** (green) - Valid checks passed
   - **Warning** (yellow) - Non-critical issues found
   - **Failed** (red) - Critical issues found

#### Step 5: Inject into CCR Blueprint
1. First, verify the assembly is valid
2. Click **"Inject to Selected CCR Blueprint"** button
3. The assembly data will be injected into your selected CCR (Configurable Car Runtime) blueprint
4. Confirmation message will appear in the log

## UI Components Explained

### Top Section
- **Select USD**: Dropdown to choose your USD stage file
- **Reload Stage**: Refresh the currently loaded USD data without restarting
- **Scan Variants**: Scan and load all available variants
- **Verify Assembly**: Validate assembly structure

### Middle Section
- **Variant Sets** (Left): Shows all available variants from the USD file
- **Current Selected Variant** (Center): Displays currently selected variant in yellow
- **Identified Parts** (Right): Lists all assembly components found

### Bottom Section
- **Inject Button**: Main action button to inject assembly into CCR blueprint
- **Statistics**: Shows count of successful operations, warnings, and failures
- **Log Output**: Timestamped log of all operations with color coding

## Log Output Interpretation

The log shows timestamped messages:
```
[23:45:12] USD stage loaded successfully.           ← Success (Green)
[23:45:13] Assembly validation found warnings.      ← Warning (Yellow)
[23:45:14] Critical assembly error detected.        ← Error (Red)
```

### Statistics Display
- **Success: 0** - Number of successful validations
- **Warning: 0** - Number of warnings encountered
- **Failed: 0** - Number of failed validations

## Troubleshooting

### "No USD stage loaded" Error
- Solution: Click "Choose USD File..." and select a valid USD file

### "No variants found" Warning
- Solution: Ensure your USD file contains variant sets defined
- Check USD file path and format

### Assembly Validation Failures
- Check part naming matches expected conventions
- Verify all required parts are present
- Review detailed log messages for specific issues

## File Support

- **USD Files**: Universal Scene Description (.usd, .usda)
- **USDZ Files**: Packaged USD (.usdz) - requires extraction
- **ASCII USD**: Plain text USD format
- **Binary USD**: Compiled USD format

## Performance Tips

1. **Large Assemblies**: Validation may take time on complex assemblies
2. **Multiple Variants**: Scanning many variants can take time - be patient
3. **Real-time Updates**: Log output updates in real-time

## Advanced Usage

### Custom Validation Rules
(Future feature) - Currently uses default validation rules

### Batch Processing
(Future feature) - Process multiple assemblies in sequence

### Assembly Comparison
(Future feature) - Compare two assembly configurations

## Technical Details

### Dependencies
- Slate UI Framework
- USD Plugin (for USD file support)
- UE5.7 or later

### Supported Platforms
- Windows (Tested)
- Linux (Should work)
- macOS (Should work)

### Memory Usage
- Minimal for typical assemblies
- Scales with USD file complexity

## Support & Issues

For issues or feature requests:
1. Check the log output for detailed error messages
2. Review this documentation for solutions
3. Report issues with detailed log output and USD file details

## Version History

### v1.0
- Initial release
- Basic USD loading and validation
- Variant scanning
- Assembly verification
- CCR blueprint injection
- Real-time logging

## License

Copyright Epic Games, Inc. All Rights Reserved.
