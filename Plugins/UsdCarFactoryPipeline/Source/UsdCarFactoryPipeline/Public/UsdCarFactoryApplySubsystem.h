#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UsdCarFactoryBlueprintTypes.h"
#include "UsdCarFactoryApplySubsystem.generated.h"

UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UUsdCarFactoryApplySubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "USD Build")
	FUsdCarFactoryApplyResult ApplyBuildPlan(const FUsdCarFactoryBuildInputs& Inputs);
};
