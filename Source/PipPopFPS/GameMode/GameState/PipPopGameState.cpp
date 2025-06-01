// Fill out your copyright notice in the Description page of Project Settings.

#include "../PipPopGameMode.h"
#include "PipPopGameState.h"


void APipPopGameState::BeginPlay()
{
    Super::BeginPlay();
    StartMatch();
}

void APipPopGameState::StartMatch()
{
    GetWorld()->GetTimerManager().SetTimer(MatchTimer, this, &APipPopGameState::EndMatch, MatchLength * 60, false);
}

void APipPopGameState::EndMatch()
{
    MatchTimer.Invalidate();
    APipPopGameMode* GameMode = (APipPopGameMode*)GetWorld()->GetAuthGameMode();
    GameMode->EndGame();
}

void APipPopGameState::UpdateMapVotes(int32 MapVoted, int32 PreviousMapVoted)
{
    MapVotes[PreviousMapVoted]--;
    MapVotes[MapVoted]++;  
}

uint8 APipPopGameState::SelectNextMap()
{
    uint8 MapIndex = 0;
    if (MapVotes.Num())
    {
        for (uint8 i = 1; i < MapVotes.Num() + 1; i++)
        {
            if (MapVotes[i] > MapVotes[MapIndex]) MapIndex = i;
        }
    }
    else
    {
        MapIndex = FMath::RandRange(0, MapVotes.Num() - 1);
    }
    return MapIndex;
}