#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "OTA_AttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class CARRENDERFACTORY_API UOTA_AttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UOTA_AttributeSet();

    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, Health)
    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, MaxHealth)
    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, Qi)
    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, MaxQi)
    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, Gold)
    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, AttackPowerMultiplier)
    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, MoveSpeedMultiplier)
    ATTRIBUTE_ACCESSORS(UOTA_AttributeSet, KillCount)

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Qi)
    FGameplayAttributeData Qi;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxQi;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Gold)
    FGameplayAttributeData Gold;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData AttackPowerMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MoveSpeedMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData KillCount;

    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_Qi(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_Gold(const FGameplayAttributeData& OldValue);
};
