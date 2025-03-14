// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PIPPOPFPS_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
    // Function to handle possession from the client
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerPossessCharacter(APawn* NewPawn);

    void ServerPossessCharacter_Implementation(APawn* NewPawn);
    bool ServerPossessCharacter_Validate(APawn* NewPawn);

    // Optional: Function to call locally (on the client side)
    void PossessCharacterOnClient(APawn* NewPawn);
};
