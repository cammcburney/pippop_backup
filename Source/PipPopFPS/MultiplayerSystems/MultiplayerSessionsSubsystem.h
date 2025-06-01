// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerCreateDelegate, bool, WasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerJoinDelegate, bool, WasSuccessful);

UCLASS()
class PIPPOPFPS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public: 
	UMultiplayerSessionsSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void CreateServer(FString ServerName);

	UFUNCTION(BlueprintCallable)
	void FindServer(FString ServerName);

	void OnCreateSessionComplete(FName SessionName, bool WasSuccessful);

	void OnDestroySessionComplete(FName SessionName, bool WasSuccessful);

	void OnFindSessionComplete(bool WasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	bool CreateServerAfterDestroy;

	FString DestroyServerName;
	FString ServerNameSearched;

	FName MySessionName;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	IOnlineSessionPtr SessionInterface;

	UPROPERTY(BlueprintAssignable)
	FServerCreateDelegate ServerCreateDel;

	UPROPERTY(BlueprintAssignable)
	FServerCreateDelegate ServerJoinDel;
};
