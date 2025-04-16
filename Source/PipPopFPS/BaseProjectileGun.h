// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGun.h"
#include "BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BaseProjectileGun.generated.h"

UCLASS()
class PIPPOPFPS_API ABaseProjectileGun : public ABaseGun
{
	GENERATED_BODY()

public:

	USceneComponent* FindTarget();

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<ABaseProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	float ProjectileScale = 1.0f;
	
	float ProjectileSpreadAngle = 35.0f;

	UFUNCTION(Server, Reliable)
	void FireMissle(AController* OwnerController, FVector Location, FRotator Rotation);

	FTimerHandle MissleFireTimer;

	bool CanFire = true;
	
	int FiredMissles = 0;
private:

	virtual void ServerBulletLineTrace(AController* OwnerController, FVector Location, FRotator Rotation) override;

	USceneComponent* HomingTarget;
};
