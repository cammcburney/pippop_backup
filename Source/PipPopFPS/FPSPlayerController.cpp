// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

// Server-side possess function
void AFPSPlayerController::ServerPossessCharacter_Implementation(APawn* NewPawn)
{
    // Ensure we have a valid pawn to possess
    if (NewPawn)
    {
        Possess(NewPawn);  // Possess the new character
    }
}

// Validate the server-side possess call (important for security in multiplayer)
bool AFPSPlayerController::ServerPossessCharacter_Validate(APawn* NewPawn)
{
    return NewPawn != nullptr;  // You can add more validation logic here
}

// Client-side function to initiate possession (calling the server RPC)
void AFPSPlayerController::PossessCharacterOnClient(APawn* NewPawn)
{
    if (HasAuthority())
    {
        Possess(NewPawn);  // If we have authority (on the server), possess directly
    }
    else
    {
        // Call the server RPC to possess the new character
        ServerPossessCharacter(NewPawn);
    }
}