#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OTA_GameMode.generated.h"

class AOTA_PlayerController;
class AOTA_PlayerState;

UCLASS()
class CARRENDERFACTORY_API AOTA_GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AOTA_GameMode();

    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartMatch();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void EndMatch();

    UFUNCTION(BlueprintPure, Category = "Game")
    bool IsMatchInProgress() const { return bMatchInProgress; }

    UFUNCTION(BlueprintPure, Category = "Game")
    int32 GetConnectedPlayersCount() const { return ConnectedPlayers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Game")
    float GetMatchTimeRemaining() const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
    int32 MaxPlayers = 4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
    float MatchDuration = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
    float WarmupDuration = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    bool bMatchInProgress = false;

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    float MatchStartTime = 0.0f;

    UPROPERTY()
    TArray<AOTA_PlayerController*> ConnectedPlayers;

    FTimerHandle MatchTimerHandle;
    FTimerHandle WarmupTimerHandle;

    void OnWarmupComplete();
    void OnMatchComplete();
    void CheckMatchEndConditions();
};
