#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "Data/CarUsdVariantDataAsset.h"
#include "CarBlueprintInjectionActor.generated.h"

UCLASS()
class USDCARFACTORYPIPELINE_API ACarBlueprintInjectionActor : public AActor
{
	GENERATED_BODY()

public:
	ACarBlueprintInjectionActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Injection")
	TObjectPtr<UCarGeneratedAssemblyDataAsset> GeneratedDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Injection")
	TObjectPtr<UCarUsdVariantDataAsset> VariantDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Injection")
	TSubclassOf<AActor> ExpectedCCRClass;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "USD Injection")
	void InjectToSelectedCCR();
};
