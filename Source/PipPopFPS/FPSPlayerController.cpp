// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PipPopGameMode.h"

void AFPSPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (GetWorld()->GetFirstLocalPlayerFromController())
    {
        ServerSpawnPlayer();
    }
}

void AFPSPlayerController::ServerSpawnPlayer_Implementation()
{
    APipPopGameMode* GameMode = Cast<APipPopGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode)
    {
        GameMode->SpawnPlayer(this);
    }
}