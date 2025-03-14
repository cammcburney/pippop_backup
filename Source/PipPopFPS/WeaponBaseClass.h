// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseClass.generated.h"

UCLASS()
class PIPPOPFPS_API AWeaponBaseClass : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeaponBaseClass();
	virtual void Tick(float DeltaTime) override;
protected:
	
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;
};
