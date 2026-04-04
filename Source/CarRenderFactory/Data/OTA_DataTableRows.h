#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OTA_DataTableRows.generated.h"

USTRUCT(BlueprintType)
struct FOTASkillDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FName SkillId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float QiCost = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float SharedCooldownSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float ExtraValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FOTAGoldGrowthRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    int32 Stack = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    int32 GoldRequired = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    float AttackMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    float QiRegenMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FOTAQiLevelRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Qi")
    float MinQiInclusive = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Qi")
    float MaxQiInclusive = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Qi")
    float AttackMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Qi")
    float MoveSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Qi")
    float QiRegenMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FOTAScoreRuleRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
    float KillWeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
    float GoldWeight = 1.0f;
};
