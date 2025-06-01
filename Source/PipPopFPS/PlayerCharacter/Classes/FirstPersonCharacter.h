// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/PlayerState.h"
#include "../State/FPSPlayerState.h"
#include "../Components/WeaponInventoryComponent.h"
#include "FirstPersonCharacter.generated.h"

class ABaseGun;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimSequence;
struct FInputActionValue;

UCLASS()
class PIPPOPFPS_API AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PickupItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WallJumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpecialAbilityOne;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchWeaponAction;



public:

	AFirstPersonCharacter();

	UPROPERTY(EditAnywhere, Category = Animation, meta = (AllowPrivateAccess = "true"))
	UAnimSequence* IdleAnimation;

	UPROPERTY(EditAnywhere, Category = Animation, meta = (AllowPrivateAccess = "true"))
	UAnimSequence* RunAnimation;

	void SetAnimation(UAnimSequence* Animation, bool Looping);
	
	UPROPERTY(VisibleAnywhere)
	class UWeaponInventoryComponent* WeaponInventory;

	TMap<int32, FKey> NumKeys;
protected:

	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Aim(const FInputActionValue& Value);
	
	void Fire(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void ServerFireGun();

	void Reload(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void ServerReloadGun();

	UFUNCTION(Server, Unreliable)
	void WallSliding();

	void WallJump(const FInputActionValue& Value);

	UFUNCTION(Server, Unreliable)
	void WallJumpRequest();
	
	void PickupItem(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void ServerPickupItem();

	void EnableReload();

	void StopAiming();

	void InitiateAbilityOne(const FInputActionValue& Value);

	
	void SwitchWeapon(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void WeaponChange();

public:	

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UPROPERTY(EditAnywhere)
	UStaticMesh *SphereMesh;

	UPROPERTY(EditAnywhere)
	UParticleSystem *MyFX;

	//Hit Points & Recieving Damage

	UFUNCTION(BlueprintCallable)
	void OnRep_UpdateHealth();

	UPROPERTY(BlueprintReadOnly)
	float MaxHealth = 1000;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateHealth, VisibleAnywhere, BlueprintReadWrite)
	float Health;
	
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(Server, Reliable)
	virtual void ServerUpdateHealth(float Damage, AFPSPlayerState* InstigatorPlayerState);

	// Movement

	UFUNCTION(Server, Reliable)
	void EnableWallJump();

	bool CanWallJump;
	bool IsWallJumpOnCooldown = false;
	FTimerHandle WallJumpCooldownHandle;
	int CurrentWallJumps = 0;
	int MaxWallJumps = 1;

	UPROPERTY(EditAnywhere)
    float WallJumpCooldownTime = 1.0f; 

	UFUNCTION(Server, Reliable)
	virtual void InitiatePrimaryAbility();

private:

	UPROPERTY(VisibleAnywhere, Category=Camera)
	UCameraComponent* CineCamera;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseGun> GunClass;

	UPROPERTY(Replicated)
	ABaseGun* Gun;

	UPROPERTY(VisibleAnywhere, Category=Camera)
	class USpringArmComponent* CameraBoom;

	FTimerHandle ReloadCooldownTimerHandle;

};
