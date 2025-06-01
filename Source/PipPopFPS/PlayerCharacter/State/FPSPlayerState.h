// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PIPPOPFPS_API AFPSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public: 

	UPROPERTY(BlueprintReadOnly, Replicated)
	int PlayerScore = 0;

	UFUNCTION(Server, Reliable)
	void UpdatePlayerScore();

	UFUNCTION(Server, Reliable)
	void ResetPlayerScore();

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
