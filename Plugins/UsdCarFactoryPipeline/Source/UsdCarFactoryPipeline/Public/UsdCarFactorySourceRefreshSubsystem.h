#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UsdCarFactoryBlueprintTypes.h"
#include "UsdCarFactorySourceRefreshSubsystem.generated.h"

UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UUsdCarFactorySourceRefreshSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "USD Build")
	FUsdCarFactorySourceRefreshResult RefreshFromSourceIfNeeded(const FUsdCarFactoryBuildInputs& Inputs);
};
