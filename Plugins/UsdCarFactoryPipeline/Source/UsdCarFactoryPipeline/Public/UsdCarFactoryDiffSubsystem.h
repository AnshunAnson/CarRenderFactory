#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UsdCarFactoryBlueprintTypes.h"
#include "UsdCarFactoryDiffSubsystem.generated.h"

UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UUsdCarFactoryDiffSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "USD Build")
	FUsdCarFactoryDiffResult BuildDiff(const FUsdCarFactoryBuildInputs& Inputs);
};
