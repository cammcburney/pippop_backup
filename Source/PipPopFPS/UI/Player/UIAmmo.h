// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "../../PlayerCharacter/Weapons/HitScan/BaseGun.h"
#include "../../PlayerCharacter/Classes/FirstPersonCharacter.h"
#include "UIAmmo.generated.h"

class UCanvasPanel;
class UHorizontalBox;
class UVerticalBox;
class UTextBlock;

UCLASS(Blueprintable)
class PIPPOPFPS_API UUIAmmo : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UUIAmmo(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UHorizontalBox* AmmoDisplay;

	UFUNCTION(BlueprintCallable)
	void UpdateAmmoDisplay();

	TArray<int32> GunData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* Panel;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* VertBox;

	void UpdateGun();

private:

	UPROPERTY()
	const APlayerController* PC;

	UPROPERTY()
    AFirstPersonCharacter* PlayerPawn;

	UPROPERTY()
	ABaseGun* CurrentBoundGun = nullptr;

	UPROPERTY()
	TArray<UWidget*> TextBlocks;
	
	UPROPERTY()
	TArray<FString> CharArray;

	void InitializeAmmoDisplay();
};