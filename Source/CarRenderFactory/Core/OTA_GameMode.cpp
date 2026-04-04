#include "OTA_GameMode.h"
#include "OTA_PlayerController.h"
#include "OTA_PlayerState.h"
#include "Character/OTA_Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/GameStateBase.h"
#include "Algo/Sort.h"

AOTA_GameMode::AOTA_GameMode()
{
    PrimaryActorTick.bCanEverTick = true;

    PlayerControllerClass = AOTA_PlayerController::StaticClass();
    PlayerStateClass = AOTA_PlayerState::StaticClass();
}

void AOTA_GameMode::BeginPlay()
{
    Super::BeginPlay();
}

void AOTA_GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (AOTA_PlayerController* OTA_PC = Cast<AOTA_PlayerController>(NewPlayer))
    {
        ConnectedPlayers.Add(OTA_PC);

        if (ConnectedPlayers.Num() >= 2 && !bMatchInProgress)
        {
            StartMatch();
        }
    }
}

void AOTA_GameMode::Logout(AController* Exiting)
{
    if (AOTA_PlayerController* OTA_PC = Cast<AOTA_PlayerController>(Exiting))
    {
        ConnectedPlayers.Remove(OTA_PC);
    }

    Super::Logout(Exiting);
}

void AOTA_GameMode::StartMatch()
{
    if (bMatchInProgress)
    {
        return;
    }

    bMatchInProgress = true;
    MatchStartTime = GetWorld()->GetTimeSeconds();

    GetWorld()->GetTimerManager().SetTimer(
        MatchTimerHandle,
        this,
        &AOTA_GameMode::OnMatchComplete,
        MatchDuration,
        false
    );
}

void AOTA_GameMode::EndMatch()
{
    if (!bMatchInProgress)
    {
        return;
    }

    bMatchInProgress = false;
    GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

    OnMatchComplete();
}

float AOTA_GameMode::GetMatchTimeRemaining() const
{
    if (!bMatchInProgress)
    {
        return 0.0f;
    }

    float ElapsedTime = GetWorld()->GetTimeSeconds() - MatchStartTime;
    return FMath::Max(0.0f, MatchDuration - ElapsedTime);
}

void AOTA_GameMode::OnWarmupComplete()
{
    StartMatch();
}

void AOTA_GameMode::OnMatchComplete()
{
    bMatchInProgress = false;
}

void AOTA_GameMode::CheckMatchEndConditions()
{
}

float AOTA_GameMode::CalculatePlayerScore(APlayerState* PlayerState) const
{
    if (!PlayerState)
    {
        return 0.0f;
    }

    const AOTA_Character* OTACharacter = Cast<AOTA_Character>(PlayerState->GetPawn());
    if (!OTACharacter)
    {
        return 0.0f;
    }

    constexpr float KillWeight = 100.0f;
    constexpr float GoldWeight = 1.0f;
    return OTACharacter->GetKillCount() * KillWeight + OTACharacter->GetGold() * GoldWeight;
}

TArray<APlayerState*> AOTA_GameMode::GetSortedLeaderboard() const
{
    TArray<APlayerState*> Result;
    if (!GameState)
    {
        return Result;
    }

    Result = GameState->PlayerArray;
    Algo::Sort(Result, [this](APlayerState* Lhs, APlayerState* Rhs)
    {
        return CalculatePlayerScore(Lhs) > CalculatePlayerScore(Rhs);
    });

    return Result;
}
