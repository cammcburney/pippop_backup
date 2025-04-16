// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "BaseGun.generated.h"

USTRUCT(BlueprintType)
struct FBulletTrajectory
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
    FVector ShotDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
    FVector End;
};

USTRUCT(BlueprintType)
struct FSpreadAngleAxis
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float UpAngle; 
	UPROPERTY(EditAnywhere)
	float DownAngle; 
	UPROPERTY(EditAnywhere)
	float LeftAngle; 
	UPROPERTY(EditAnywhere)
	float RightAngle; 
};

UCLASS()
class PIPPOPFPS_API ABaseGun : public AActor
{
	GENERATED_BODY()
	
public:	

	ABaseGun();
	
	UFUNCTION(Server, Reliable)
	void FireGun();
	
	bool CanFire();

	UFUNCTION(Server, Reliable)
	virtual void ServerBulletLineTrace(AController* OwnerController, FVector Location, FRotator Rotation);

	UFUNCTION(Server, Reliable)
	void DamageActors(AController* OwnerController, FHitResult Hit, FBulletTrajectory Bullet);
	
	FBulletTrajectory CalculateBulletTrajectory(FVector Location, FRotator Rotation);

	UFUNCTION(Server, Reliable)
	void Reload();

	UFUNCTION(Server, Reliable)
	void ReloadStatus(bool CanReload);
	
	bool ReloadAvailable = true;

	UFUNCTION(NetMulticast, Unreliable)
	void SpawnBulletImpact(FHitResult Hit, FBulletTrajectory Bullet);

	UFUNCTION(NetMulticast, Unreliable)
	void SpawnMuzzleFlash();

protected:
	
	virtual void BeginPlay() override;


public:	
	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh;
	
private:
	
	AController* GetOwnerController();

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* MuzzleFlashNiagaraSystem;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* BulletImpactNiagaraSystem;


	UPROPERTY(EditAnywhere)
	float MaxRange = 2000;

	UPROPERTY(EditAnywhere, Replicated)
	float Damage = 111;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere)
	float FireRate = 0.5f;
	
	UPROPERTY(EditAnywhere)
	FSpreadAngleAxis SpreadAngles;

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo = 8;

	UPROPERTY()
	int32 Ammo = 0;

	UPROPERTY(EditAnywhere)
	int32 Magazines = 3;

	UPROPERTY(EditAnywhere)
	int32 BulletsFiredPerShot = 1;

	float LastFireTime = 0.0f;
};
