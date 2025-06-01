// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerState.h"
#include "Net/UnrealNetwork.h"

void AFPSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFPSPlayerState, PlayerScore);
}

void AFPSPlayerState::UpdatePlayerScore_Implementation()
{
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, *FString::Printf(TEXT("Updating Player Score: %d"), PlayerScore));
    PlayerScore++;
}

void AFPSPlayerState::ResetPlayerScore_Implementation()
{
    PlayerScore = 0;
}