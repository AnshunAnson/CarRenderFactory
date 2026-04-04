#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OTA_InputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS(BlueprintType)
class CARRENDERFACTORY_API UOTA_InputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    UInputAction* MoveAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    UInputAction* LookAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    UInputAction* JumpAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    UInputAction* LightAttackAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    UInputAction* ParryAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    UInputAction* HeavyAttackAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    UInputAction* Skill1Action = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    UInputAction* Skill2Action = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    UInputAction* Skill3Action = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* CombatMappingContext = nullptr;
};
