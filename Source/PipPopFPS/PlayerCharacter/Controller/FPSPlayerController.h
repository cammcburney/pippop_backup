// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "../Classes/FirstPersonCharacter.h"
#include "FPSPlayerController.generated.h"

UCLASS(BlueprintType)
class PIPPOPFPS_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AFirstPersonCharacter> PlayerClass = AFirstPersonCharacter::StaticClass();
    
protected:

    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerSpawnPlayer();

    UFUNCTION(Server, Reliable)
    void SetCharacter(AFirstPersonCharacter* NewPlayerCharacter);
    
};
