// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "FPSPlayerController.generated.h"

UCLASS()
class PIPPOPFPS_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerSpawnPlayer();

    

};
