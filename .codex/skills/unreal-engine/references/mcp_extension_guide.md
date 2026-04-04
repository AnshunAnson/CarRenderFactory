# McpAutomationBridge Handler 扩展指南

## 概述

本文档说明如何在 McpAutomationBridge 插件中添加新的 Handler，以扩展 MCP 自动化能力。

## Handler 添加步骤

### 1. 声明处理函数

在 `McpAutomationBridgeSubsystem.h` 中声明处理函数：

```cpp
// 在类声明的 private 或 public 区域添加
bool HandleMyCustomAction(const FString &RequestId,
                         const FString &Action,
                         const TSharedPtr<FJsonObject> &Payload,
                         TSharedPtr<FMcpBridgeWebSocket> RequestingSocket);
```

### 2. 实现处理函数

创建新的 Handler 文件或添加到现有文件：

```cpp
// McpAutomationBridge_MyCustomHandlers.cpp

#include "McpAutomationBridgeSubsystem.h"
#include "McpHandlerUtils.h"
#include "Dom/JsonObject.h"

bool UMcpAutomationBridgeSubsystem::HandleMyCustomAction(
    const FString &RequestId,
    const FString &Action,
    const TSharedPtr<FJsonObject> &Payload,
    TSharedPtr<FMcpBridgeWebSocket> RequestingSocket)
{
    // 1. 解析参数
    FString AssetPath;
    if (!McpHandlerUtils::GetString(Payload, TEXT("asset_path"), AssetPath))
    {
        SendAutomationError(RequestingSocket, RequestId,
            TEXT("Missing required parameter: asset_path"),
            TEXT("INVALID_PARAMETERS"));
        return true;
    }

    // 2. 执行 UE 操作
    // ... 你的逻辑代码 ...

    // 3. 构建结果
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("status"), TEXT("success"));
    Result->SetStringField(TEXT("asset_path"), AssetPath);

    // 4. 发送成功响应
    SendAutomationResponse(RequestingSocket, RequestId, true,
        TEXT("Operation completed successfully"), Result);
    return true;
}
```

### 3. 注册 Handler

在 `McpAutomationBridgeSubsystem.cpp` 的 `InitializeHandlers()` 函数中注册：

```cpp
void UMcpAutomationBridgeSubsystem::InitializeHandlers()
{
    // ... 其他 handler 注册 ...

    // 添加新的 handler 注册
    RegisterHandler(TEXT("my_custom_action"),
        [this](const FString &R, const FString &A,
               const TSharedPtr<FJsonObject> &P,
               TSharedPtr<FMcpBridgeWebSocket> S) {
            return HandleMyCustomAction(R, A, P, S);
        });
}
```

## Handler 签名详解

```cpp
using FAutomationHandler = TFunction<bool(
    const FString &RequestId,    // 请求 ID，用于关联响应
    const FString &Action,        // Action 名称
    const TSharedPtr<FJsonObject> &Payload, // 请求参数
    TSharedPtr<FMcpBridgeWebSocket> RequestingSocket // WebSocket 连接
)>;
```

## 响应机制

### 成功响应
```cpp
void SendAutomationResponse(
    TSharedPtr<FMcpBridgeWebSocket> TargetSocket,
    const FString &RequestId,
    bool bSuccess,
    const FString &Message,
    const TSharedPtr<FJsonObject> &Result = nullptr,
    const FString &ErrorCode = FString());
```

### 错误响应
```cpp
void SendAutomationError(
    TSharedPtr<FMcpBridgeWebSocket> TargetSocket,
    const FString &RequestId,
    const FString &Message,
    const FString &ErrorCode);
```

## 常用工具函数

### McpHandlerUtils

```cpp
// 获取字符串参数
bool GetString(const TSharedPtr<FJsonObject>& Payload, const FString& Key, FString& OutValue);

// 获取数值参数
bool GetInt32(const TSharedPtr<FJsonObject>& Payload, const FString& Key, int32& OutValue);
bool GetFloat(const TSharedPtr<FJsonObject>& Payload, const FString& Key, float& OutValue);

// 获取布尔参数
bool GetBool(const TSharedPtr<FJsonObject>& Payload, const FString& Key, bool& OutValue);

// 获取对象参数
bool GetObject(const TSharedPtr<FJsonObject>& Payload, const FString& Key, TSharedPtr<FJsonObject>& OutValue);

// 获取数组参数
bool GetStringArray(const TSharedPtr<FJsonObject>& Payload, const FString& Key, TArray<FString>& OutArray);
```

### McpAutomationBridgeHelpers

```cpp
// 安全保存资产
bool McpSafeAssetSave(UObject* Asset);

// 路径清理
FString SanitizeProjectRelativePath(const FString& Path);

// 查找 Actor
AActor* FindActorByName(const FString& Target, bool bExactMatchOnly = false);
AActor* FindActorByTag(const FString& Tag);
```

## 代码示例

### 示例 1：创建简单的 Actor 操作

```cpp
bool UMcpAutomationBridgeSubsystem::HandleSpawnActorAtLocation(
    const FString &RequestId,
    const FString &Action,
    const TSharedPtr<FJsonObject> &Payload,
    TSharedPtr<FMcpBridgeWebSocket> RequestingSocket)
{
    FString AssetPath;
    FVector Location(0, 0, 0);
    FRotator Rotation(0, 0, 0);

    if (!McpHandlerUtils::GetString(Payload, TEXT("asset_path"), AssetPath))
    {
        SendAutomationError(RequestingSocket, RequestId,
            TEXT("Missing asset_path"), TEXT("INVALID_PARAMETERS"));
        return true;
    }

    // 解析位置
    TSharedPtr<FJsonObject> LocationObj;
    if (McpHandlerUtils::GetObject(Payload, TEXT("location"), LocationObj))
    {
        McpHandlerUtils::GetFloat(LocationObj, TEXT("x"), Location.X);
        McpHandlerUtils::GetFloat(LocationObj, TEXT("y"), Location.Y);
        McpHandlerUtils::GetFloat(LocationObj, TEXT("z"), Location.Z);
    }

    // 加载蓝图
    FSoftObjectPtr AssetPtr(AssetPath);
    if (!AssetPtr.IsValid())
    {
        SendAutomationError(RequestingSocket, RequestId,
            TEXT("Failed to load asset: ") + AssetPath, TEXT("ASSET_LOAD_FAILED"));
        return true;
    }

    // 生成 Actor
    UWorld* World = GEditor->GetEditorWorldContext().World();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* SpawnedActor = World->SpawnActor(AssetPtr.Get(), &Location, &Rotation, SpawnParams);

    // 返回结果
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("actor_name"), SpawnedActor->GetName());
    Result->SetStringField(TEXT("actor_path"), SpawnedActor->GetPathName());

    SendAutomationResponse(RequestingSocket, RequestId, true,
        TEXT("Actor spawned successfully"), Result);
    return true;
}
```

### 示例 2：异步操作（带进度）

```cpp
bool UMcpAutomationBridgeSubsystem::HandleLongRunningOperation(
    const FString &RequestId,
    const FString &Action,
    const TSharedPtr<FJsonObject> &Payload,
    TSharedPtr<FMcpBridgeWebSocket> RequestingSocket)
{
    // 先发送初始响应
    SendAutomationResponse(RequestingSocket, RequestId, true,
        TEXT("Operation started"), nullptr);

    // 在后台线程执行
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, RequestId, RequestingSocket]()
    {
        // 模拟长时间操作
        for (int32 i = 0; i < 10; ++i)
        {
            // 发送进度更新
            AsyncTask(ENamedThreads::GameThread, [this, RequestId, i, RequestingSocket]()
            {
                SendProgressUpdate(RequestId, i * 10, FString::Printf(TEXT("Processing step %d..."), i));
            });

            FPlatformProcess::Sleep(0.5f);
        }

        // 完成
        AsyncTask(ENamedThreads::GameThread, [this, RequestId, RequestingSocket]()
        {
            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("status"), TEXT("completed"));

            SendAutomationResponse(RequestingSocket, RequestId, true,
                TEXT("Operation completed"), Result);
        });
    });

    return true;
}
```

## 最佳实践

### 1. 参数验证
```cpp
// 总是检查必要参数
if (!Payload->HasField(TEXT("required_param")))
{
    SendAutomationError(Socket, RequestId, TEXT("Missing required_param"), TEXT("INVALID_PARAMETERS"));
    return true;
}
```

### 2. 错误处理
```cpp
// 使用 try-catch 捕获异常
// 但不要捕获 ensure/panic
```

### 3. 线程安全
```cpp
// UI 操作必须在游戏线程
// 使用 AsyncTask 切换线程
```

### 4. 资源清理
```cpp
// 使用 ScopeExit 确保清理
ON_SCOPE_EXIT {
    // 清理代码
};
```

### 5. 日志记录
```cpp
UE_LOG(LogMcpAutomationBridgeSubsystem, Verbose,
    TEXT("HandleMyAction: Processing request %s"), *RequestId);
```

## 文件结构建议

```
Source/McpAutomationBridge/Private/
├── McpAutomationBridge_MyFeatureHandlers.cpp   # 新 Handler 实现
├── McpAutomationBridge_MyFeatureHandlers.h     # (可选) 辅助函数
```

或者添加到现有文件：
- `McpAutomationBridge_ControlHandlers.cpp` - Actor/编辑器控制
- `McpAutomationBridge_AssetWorkflowHandlers.cpp` - 资产管理
- `McpAutomationBridge_BlueprintHandlers.cpp` - 蓝图操作

## 版本兼容性

使用 `McpVersionCompatibility.h` 处理 UE 版本差异：

```cpp
#include "McpVersionCompatibility.h"

// 检查 UE 版本
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
    // UE 5.3+ 特定代码
#endif
```

## 调试技巧

1. **启用详细日志**：
   ```cpp
   UE_LOG(LogMcpAutomationBridgeSubsystem, Verbose, TEXT("..."));
   ```

2. **使用断点**：在 `ProcessAutomationRequest` 中设置断点

3. **测试 Handler**：直接调用 Handler 函数进行单元测试

## See Also

- [MCP 工具参考](mcp_tools.md)
- [McpAutomationBridge 源码](../Plugins/McpAutomationBridge)
- [UE5 API 文档](https://docs.unrealengine.com)