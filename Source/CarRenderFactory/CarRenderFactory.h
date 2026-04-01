// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// ============================================================
// 预编译头保护
// ============================================================
// #pragma once 是现代 C++ 的头文件保护方式
// 确保此头文件在同一编译单元中只被包含一次
//
// 与传统的 #ifndef/#define/#endif 相比:
// 优点:
// - 更简洁，不易出错
// - 编译速度更快 (某些编译器优化)
//
// 缺点:
// - 不是 C++ 标准，但所有主流编译器都支持
// - 无法处理重复包含需要多次处理的情况
#pragma once

// ============================================================
// 核心头文件包含
// ============================================================
// CoreMinimal.h 是 UE 的核心最小头文件集合
// 包含了最常用的 UE 类型定义:
//
// 基础类型:
// - uint8, int32, int64, float, double 等跨平台类型
// - TCHAR (跨平台字符类型)
// - bool, SIZE_T 等
//
// 核心容器:
// - TArray - 动态数组
// - TMap - 映射表
// - TSet - 集合
// - FString - 字符串
// - FName - 名称
// - FText - 本地化文本
//
// 数学类型:
// - FVector - 3D 向量
// - FRotator - 旋转
// - FQuat - 四元数
// - FTransform - 变换
// - FMatrix - 矩阵
//
// 智能指针:
// - TSharedPtr - 共享指针
// - TWeakPtr - 弱指针
// - TUniquePtr - 独占指针
//
// 其他:
// - TDelegate - 委托
// - TEnumAsByte - 枚举包装
// - 各种宏定义 (check, ensure, UE_LOG 等)
#include "CoreMinimal.h"

// ============================================================
// 模块声明区域
// ============================================================
// 当前模块头文件为空，仅包含最小必要头文件
//
// 后续可在此添加:
//
// 1. 前向声明:
//    class AMyActor;
//    class UMyComponent;
//
// 2. 模块级宏定义:
//    #define MY_MODULE_API __declspec(dllexport)
//
// 3. 函数声明:
//    CARRENDERFACTORY_API void MyModuleFunction();
//
// 4. 日志分类声明:
//    DECLARE_LOG_CATEGORY_EXTERN(LogCarRenderFactory, Log, All);
//
// 5. 自治类型前向声明:
//    struct FMyStruct;
//
// ============================================================
// 示例: 添加日志分类
// ============================================================
// 日志分类声明 - 用于模块内的日志输出
DECLARE_LOG_CATEGORY_EXTERN(LogCarRenderFactory, Log, All);
//
// 使用方式 (在 .cpp 中):
// DEFINE_LOG_CATEGORY(LogCarRenderFactory);
// UE_LOG(LogCarRenderFactory, Log, TEXT("Message: %s"), *SomeString);
//
// ============================================================
// 示例: 模块导出宏
// ============================================================
// 如果需要从模块导出函数/类供其他模块使用:
//
// #ifdef CARRENDERFACTORY_EXPORTS
//     #define CARRENDERFACTORY_API __declspec(dllexport)
// #else
//     #define CARRENDERFACTORY_API __declspec(dllimport)
// #endif
//
// ============================================================
// 示例: 前向声明常用类
// ============================================================
// // 前向声明可以减少头文件依赖，加快编译速度
// class AActor;
// class UActorComponent;
// class UWorld;
