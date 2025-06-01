#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../PlayerCharacter/Controller/FPSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PipPopGameMode.generated.h"


/**
 * 
 */
UCLASS()
class PIPPOPFPS_API APipPopGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	TMap<int32, FString> MapList;

	APipPopGameMode();
	
	UPROPERTY(BlueprintReadOnly, Category = "GamePlayers")
	TArray<class APlayerController*> ConnectedPlayers;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SpawnPlayer(AFPSPlayerController* PC, FName SpawnTag);

	void EndGame();

	UPROPERTY(EditAnywhere)
	int32 MapSelectTimerLength = 15;

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

	virtual void Logout(AController* Exiting) override;

	FTransform FindRandomPlayerStart(FName Tag);

	UFUNCTION(BlueprintCallable)
	void HostLANGame();
	
	UFUNCTION(BlueprintCallable)
	void JoinLANGame();

private:

	FTimerHandle MapVoteHandle;

	void LoadMap();


};
