#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PipPopGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PIPPOPFPS_API APipPopGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	APipPopGameMode();

	UFUNCTION(BlueprintCallable)
	void HostLANGame();
	
	UFUNCTION(BlueprintCallable)
	void JoinLANGame();

	UFUNCTION(Server, Reliable)
	void SpawnPlayer(APlayerController* PC);
};
