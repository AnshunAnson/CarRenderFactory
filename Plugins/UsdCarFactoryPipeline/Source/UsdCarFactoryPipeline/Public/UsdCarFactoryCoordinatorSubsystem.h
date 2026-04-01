#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UsdCarFactoryBlueprintTypes.h"
#include "UsdCarFactoryCoordinatorSubsystem.generated.h"

UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UUsdCarFactoryCoordinatorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "USD Build")
	FUsdCarFactoryBuildResult RunBuild(const FUsdCarFactoryBuildInputs& Inputs);
};
