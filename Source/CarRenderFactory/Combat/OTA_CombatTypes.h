#pragma once

#include "CoreMinimal.h"
#include "OTA_CombatTypes.generated.h"

UENUM(BlueprintType)
enum class EMeleeType : uint8
{
    None    UMETA(DisplayName = "无"),
    White   UMETA(DisplayName = "白刀 - 普通攻击"),
    Red     UMETA(DisplayName = "红刀 - 振刀架势"),
    Blue    UMETA(DisplayName = "蓝刀 - 霸体攻击"),
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle            UMETA(DisplayName = "空闲"),
    Attacking       UMETA(DisplayName = "攻击中"),
    Parrying        UMETA(DisplayName = "振刀中"),
    HeavyAttacking  UMETA(DisplayName = "霸体攻击中"),
    Stunned         UMETA(DisplayName = "硬直"),
    Dead            UMETA(DisplayName = "死亡"),
    Looting         UMETA(DisplayName = "摸金中"),
};

UENUM(BlueprintType)
enum class ECounterResult : uint8
{
    None        UMETA(DisplayName = "无克制"),
    Counter     UMETA(DisplayName = "克制成功"),
    Countered   UMETA(DisplayName = "被克制"),
    Draw        UMETA(DisplayName = "平局"),
};

UENUM(BlueprintType)
enum class EBulletType : uint8
{
    None    UMETA(DisplayName = "无"),
    White   UMETA(DisplayName = "白弹 - 普通弹"),
    Red     UMETA(DisplayName = "红弹 - 散射弹"),
    Blue    UMETA(DisplayName = "蓝弹 - 穿甲弹"),
};

UENUM(BlueprintType)
enum class EDeflectionResult : uint8
{
    Failed      UMETA(DisplayName = "偏转失败"),
    Partial     UMETA(DisplayName = "部分偏转"),
    Deflected   UMETA(DisplayName = "偏转成功"),
    Reflected   UMETA(DisplayName = "完美偏转"),
};

USTRUCT(BlueprintType)
struct FFrameData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 StartupFrames = 8;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 ActiveFrames = 4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 RecoveryFrames = 12;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 PerfectWindowFrames = 3;

    float GetStartupTime() const { return StartupFrames / 60.0f; }
    float GetActiveTime() const { return ActiveFrames / 60.0f; }
    float GetRecoveryTime() const { return RecoveryFrames / 60.0f; }
    float GetPerfectWindowTime() const { return PerfectWindowFrames / 60.0f; }
    float GetTotalTime() const { return (StartupFrames + ActiveFrames + RecoveryFrames) / 60.0f; }
};

namespace OTA_FramePresets
{
    inline FFrameData LightAttack()
    {
        FFrameData Data;
        Data.StartupFrames = 8;
        Data.ActiveFrames = 4;
        Data.RecoveryFrames = 12;
        return Data;
    }

    inline FFrameData Parry()
    {
        FFrameData Data;
        Data.StartupFrames = 4;
        Data.ActiveFrames = 9;
        Data.RecoveryFrames = 18;
        Data.PerfectWindowFrames = 3;
        return Data;
    }

    inline FFrameData HeavyAttack()
    {
        FFrameData Data;
        Data.StartupFrames = 16;
        Data.ActiveFrames = 8;
        Data.RecoveryFrames = 20;
        return Data;
    }
}
