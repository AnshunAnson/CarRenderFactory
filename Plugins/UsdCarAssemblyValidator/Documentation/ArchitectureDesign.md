# USD Car Assembly Validator - Architecture Design

## System Architecture

### Component Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    UE5 Editor                               │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │       FUsdCarAssemblyValidatorModule (Main)          │  │
│  │       - Module lifecycle management                  │  │
│  │       - Menu registration                            │  │
│  └────────────────┬─────────────────────────────────────┘  │
│                   │                                         │
│  ┌────────────────┴─────────────────────────────────────┐  │
│  │    FAssemblyValidatorUIManager                       │  │
│  │    - UI lifecycle                                    │  │
│  │    - Event handling                                  │  │
│  │    - Data management                                 │  │
│  └────────────────┬─────────────────────────────────────┘  │
│                   │                                         │
│  ┌────────────────┴─────────────────────────────────────┐  │
│  │         SAssemblyValidatorPanel (Slate Widget)       │  │
│  │         - UI rendering and layout                    │  │
│  │         - User interaction                           │  │
│  │         - Real-time updates                          │  │
│  └────────────────┬─────────────────────────────────────┘  │
│                   │                                         │
│        ┌──────────┴──────────┬───────────────┐             │
│        │                     │               │             │
│  ┌─────┴──────┐      ┌───────┴──────┐  ┌────┴─────┐      │
│  │ USD Loader │      │  Validator   │  │CCR Helper │      │
│  │            │      │              │  │           │      │
│  │ - Load     │      │ - Verify     │  │ - Inject  │      │
│  │ - Parse    │      │ - Validate   │  │ - Update  │      │
│  │ - Extract  │      │ - Report     │  │ - Confirm │      │
│  └────────────┘      └──────────────┘  └───────────┘      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## Data Flow

### 1. User Load USD File
```
User clicks "Choose USD File"
    ↓
File Browser opens
    ↓
User selects USD file
    ↓
FAssemblyValidatorUIManager::LoadUsdStage()
    ↓
Parse USD file structure
    ↓
Extract variant sets and parts
    ↓
Update SAssemblyValidatorPanel
    ↓
Display results in UI
    ↓
Add log entry
```

### 2. Variant Scanning
```
User clicks "Scan Variants"
    ↓
FAssemblyValidatorUIManager::ScanVariants()
    ↓
Query FUsdStageData
    ↓
Retrieve variant list
    ↓
Update VariantSetsTreeView
    ↓
Add log entry
```

### 3. Assembly Validation
```
User clicks "Verify Assembly"
    ↓
FAssemblyValidatorUIManager::VerifyAssembly()
    ↓
Create FAssemblyValidationResult
    ↓
Run validators:
  - Part hierarchy check
  - Naming convention check
  - Relationship validation
  - Attribute validation
    ↓
Update statistics counters
    ↓
Log results with color coding
```

### 4. CCR Blueprint Injection
```
User clicks "Inject to CCR Blueprint"
    ↓
FAssemblyValidatorUIManager::InjectToCCRBlueprint()
    ↓
Validate assembly is ready
    ↓
Create CCR blueprint if needed
    ↓
Inject assembly data:
  - Parts hierarchy
  - Relationships
  - Attributes
    ↓
Compile blueprint
    ↓
Log success/failure
```

## Class Hierarchy

### Module Class
```cpp
class FUsdCarAssemblyValidatorModule : public IModuleInterface
{
    void StartupModule() override;
    void ShutdownModule() override;
    void PluginButtonClicked();
    void RegisterMenus();
    TSharedRef<SDockTab> OnSpawnPluginTab(...);
    
private:
    TSharedPtr<FUICommandList> PluginCommands;
};
```

### UI Manager Class
```cpp
class FAssemblyValidatorUIManager : public TSharedFromThis<...>
{
public:
    void Initialize();
    void Shutdown();
    TSharedPtr<SAssemblyValidatorPanel> GetUIPanel() const;
    void ShowValidatorWindow();
    void HideValidatorWindow();
    void LoadUsdStage(const FString& FilePath);
    void ScanVariants();
    void VerifyAssembly();
    void InjectToCCRBlueprint();
    void AddLogMessage(...);
    
private:
    TSharedPtr<SAssemblyValidatorPanel> UIPanel;
    TWeakPtr<SWindow> ValidatorWindow;
    FUsdStageData* CurrentStageData;
};
```

### UI Widget Class
```cpp
class SAssemblyValidatorPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(...) {}
    SLATE_END_ARGS()
    
    void Construct(const FArguments& InArgs);
    void AddLogMessage(...);
    
private:
    // UI Components
    TSharedPtr<SComboButton> UsdFileButton;
    TSharedPtr<SButton> ReloadStageButton;
    TSharedPtr<SButton> ScanVariantsButton;
    TSharedPtr<SButton> VerifyAssemblyButton;
    TSharedPtr<STreeView<...>> VariantSetsTreeView;
    TSharedPtr<STextBlock> CurrentVariantTextBlock;
    TSharedPtr<SListView<...>> IdentifiedPartsListView;
    TSharedPtr<SButton> InjectToCCRButton;
    TSharedPtr<SMultiLineEditableTextBox> LogOutputTextBox;
    
    // Callbacks
    FReply OnReloadStage();
    FReply OnScanVariants();
    FReply OnVerifyAssembly();
    FReply OnInjectToCCR();
};
```

## Data Structures

### FUsdStageData
```cpp
struct FUsdStageData
{
    FString StageFilePath;
    TArray<FString> VariantSets;
    TArray<FString> IdentifiedParts;
    FString CurrentVariant;
};
```

### FAssemblyValidationResult
```cpp
struct FAssemblyValidationResult
{
    bool bIsValid;
    int32 WarningCount;
    int32 ErrorCount;
    TArray<FString> ValidationMessages;
};
```

## Event Flow

### Button Press Event
```
SButton::OnClicked
    ↓
OnVerifyAssembly() [FReply]
    ↓
FAssemblyValidatorUIManager::VerifyAssembly()
    ↓
Perform validation
    ↓
AddLogMessage() [void]
    ↓
SAssemblyValidatorPanel::AddLogMessage()
    ↓
Update LogOutputTextBox text
    ↓
UI refreshes automatically
```

## Thread Safety

### Current Implementation
- All operations on main thread (Slate requirement)
- UI updates automatic (binding system)
- No multithreading currently

### Future Considerations
- Async USD file loading (long files)
- Async validation (complex assemblies)
- Worker threads with Slate thread communication

## Memory Management

### Ownership
- `FUsdCarAssemblyValidatorModule`: Owns `FAssemblyValidatorUIManager`
- `FAssemblyValidatorUIManager`: Owns `SAssemblyValidatorPanel` and `FUsdStageData`
- `SAssemblyValidatorPanel`: Manages UI widget lifetime

### Cleanup
- Module shutdown cleans up all owned objects
- Smart pointers handle cleanup automatically
- Window destruction handled by Slate

## Performance Considerations

### USD File Loading
- Lazy loading of variants (on-demand)
- Caching of parsed data
- Large file support (streaming)

### Validation
- Incremental validation possible
- Early exit on critical errors
- Progress indication for long operations

### UI Rendering
- Virtualized list views (only visible items)
- Efficient splitter layout
- Minimal redraws on updates

## Extension Points

### Adding New Validators
1. Create validator function
2. Call from `VerifyAssembly()`
3. Log results

### Adding New UI Panels
1. Extend `SAssemblyValidatorPanel`
2. Add new Slate widgets
3. Wire up callbacks

### Adding New File Formats
1. Create loader class
2. Register with file browser
3. Parse into `FUsdStageData`

## Error Handling

### Strategy
- User-friendly error messages
- Detailed logs for debugging
- Graceful degradation

### Example Error Flow
```
Operation fails
    ↓
Create error message
    ↓
Log with error flag (red)
    ↓
Update statistics
    ↓
Display in UI
    ↓
User can see what went wrong
```

## Testing Strategy

### Unit Tests
- Validator logic
- Data structure operations
- File parsing

### Integration Tests
- UI interaction
- USD loading
- CCR injection

### Manual Tests
- Full workflow
- Large assemblies
- Multiple variants
- Error conditions
