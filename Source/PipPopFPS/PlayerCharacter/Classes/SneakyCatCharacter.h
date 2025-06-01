// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FirstPersonCharacter.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SneakyCatCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PIPPOPFPS_API ASneakyCatCharacter : public AFirstPersonCharacter
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Materials")
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Materials")
	UMaterialInterface* HiddenMaterial;

	UPROPERTY(BlueprintReadWrite, Category = "Materials")
	UMaterialInstanceDynamic* MI;

	
protected:

	virtual void InitiatePrimaryAbility_Implementation() override;

	void EndSneakAbility();

private:

	FTimerHandle PrimaryAbilityCooldownHandle; 
};
