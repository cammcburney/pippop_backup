// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PipPopGameState.generated.h"

/**
 * 
 */
UCLASS()
class PIPPOPFPS_API APipPopGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<int32> MapVotes;

	UPROPERTY(EditAnywhere)
	float MatchLength = 5.00f;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle MatchTimer;
	
	void UpdateMapVotes(int32 MapVoted, int32 PreviousMapVoted);

	uint8 SelectNextMap();

protected:

	virtual void BeginPlay() override;

private:

	void StartMatch();

	void EndMatch();
};
