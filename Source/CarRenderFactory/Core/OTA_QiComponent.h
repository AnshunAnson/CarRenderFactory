#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OTA_QiComponent.generated.h"

class AOTA_Character;

UENUM(BlueprintType)
enum class EQiLevel : uint8
{
    High,       // 80-100: 攻击力+10%，气势恢复+20%
    Normal,     // 50-79: 正常状态
    Low,        // 20-49: 攻击力-10%，移动速度-5%
    Critical    // 0-19: 攻击力-20%，无法使用技能
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQiLevelChanged, EQiLevel, OldLevel, EQiLevel, NewLevel);

UCLASS(ClassGroup = "Combat", meta = (BlueprintSpawnableComponent))
class CARRENDERFACTORY_API UOTA_QiComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOTA_QiComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintPure, Category = "Qi")
    EQiLevel GetQiLevel() const;

    UFUNCTION(BlueprintPure, Category = "Qi")
    float GetQiValue() const;

    UFUNCTION(BlueprintCallable, Category = "Qi")
    void ModifyQi(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Qi")
    void OnHitDealt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Qi")
    void OnHitReceived(AActor* Attacker);

    UFUNCTION(BlueprintCallable, Category = "Qi")
    void OnBlockSuccess(AActor* Attacker);

    UFUNCTION(BlueprintCallable, Category = "Qi")
    void OnLootStart();

    UFUNCTION(BlueprintCallable, Category = "Qi")
    void OnLootEnd();

    UPROPERTY(BlueprintAssignable, Category = "Qi")
    FOnQiLevelChanged OnQiLevelChanged;

protected:
    static constexpr float QI_HIGH_THRESHOLD = 80.0f;
    static constexpr float QI_LOW_THRESHOLD = 20.0f;

    static constexpr float QI_GAIN_ON_HIT = 5.0f;
    static constexpr float QI_GAIN_ON_BLOCK = 3.0f;
    static constexpr float QI_LOSS_ON_HIT = 8.0f;
    static constexpr float QI_LOSS_PER_SECOND_LOOTING = 10.0f;
    static constexpr float QI_REGEN_RATE = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Qi")
    bool bIsLooting = false;

private:
    AOTA_Character* OwnerCharacter = nullptr;
    EQiLevel CurrentQiLevel = EQiLevel::Normal;
    float LootQiDrainAccumulator = 0.0f;

    EQiLevel CalculateQiLevel(float QiValue) const;
    void UpdateQiLevelEffects(EQiLevel OldLevel, EQiLevel NewLevel);
};
