#include "FirstPersonCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/DamageType.h"
#include "../Weapons/HitScan/BaseGun.h"
#include "DrawDebugHelpers.h"

AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bUseControllerRotationYaw = true;
	CanWallJump = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	CineCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CineCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    CineCamera->bUsePawnControlRotation = false; 
    CineCamera->FieldOfView = 110.0f;

	WeaponInventory = CreateDefaultSubobject<UWeaponInventoryComponent>(TEXT("WeaponInventory"));
	WeaponInventory->SetIsReplicated(true);

	NumKeys.Add(1, EKeys::One);
	NumKeys.Add(2, EKeys::Two);
	NumKeys.Add(3, EKeys::Three);
	NumKeys.Add(4, EKeys::Four);
	NumKeys.Add(5, EKeys::Five);
	NumKeys.Add(6, EKeys::Six);
	NumKeys.Add(7, EKeys::Seven);
	NumKeys.Add(8, EKeys::Eight);
	NumKeys.Add(9, EKeys::Nine);
}

void AFirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);

	Health = MaxHealth;

	if (HasAuthority() && !Gun)
    {
        Gun = GetWorld()->SpawnActor<ABaseGun>(GunClass);
        if (Gun)
        {   
            Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
            Gun->SetOwner(this);
        }
    }

	SetAnimation(IdleAnimation, true);
}

void AFirstPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UCharacterMovementComponent* MoveComponent = GetCharacterMovement();

	if (MoveComponent->IsMovingOnGround())
	{
		CanWallJump = false;
		CurrentWallJumps = 0;
	}
	if (!CanWallJump)
	{
		WallSliding();
	}
}

void AFirstPersonCharacter::SetAnimation(UAnimSequence* Animation, bool Looping)
{
	GetMesh()->PlayAnimation(Animation, Looping);
}

void AFirstPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{	
		// Setup all input bindings (moving, aiming, firing, etc.)
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(WallJumpAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::WallJump);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::Look);

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AFirstPersonCharacter::StopAiming);

		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::Fire);

		//Reloading
        EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::Reload);

		//Picking up items
		EnhancedInputComponent->BindAction(PickupItemAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::PickupItem);

		//Use Primary Special Ability
		EnhancedInputComponent->BindAction(SpecialAbilityOne, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::InitiateAbilityOne);
		
		//Switch Weapons
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::SwitchWeapon);
	}
}

void AFirstPersonCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{	
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
		SetAnimation(RunAnimation, true);
	}
}

void AFirstPersonCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void AFirstPersonCharacter::WallSliding_Implementation()
{
    UCharacterMovementComponent* MoveComponent = GetCharacterMovement();
    if (MoveComponent->IsFalling())
    {
        FVector Start = GetActorLocation();
        FVector End = Start + GetActorForwardVector() * 100;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		FHitResult Hit;
		bool NearWall = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel2, QueryParams);

        if (NearWall && Hit.GetActor())
        {

            AActor* HitActor = Hit.GetActor();
		
            if (HitActor->ActorHasTag("Wall"))
            {
                FRotator NewRotation = GetActorRotation();
                NewRotation.Yaw = FMath::RadiansToDegrees(FMath::Atan2(Hit.ImpactNormal.Y, Hit.ImpactNormal.X)) + 180.0f;
                SetActorRotation(NewRotation);

                FVector NewVelocity = FMath::VInterpConstantTo(MoveComponent->Velocity, FVector(0.0f, 0.0f, 0.0f), GetWorld()->GetDeltaSeconds(), 500);
                MoveComponent->Velocity = NewVelocity;

                CanWallJump = true;
            }
        }
    }
}


void AFirstPersonCharacter::EnableWallJump_Implementation()
{
	IsWallJumpOnCooldown = false;
}

void AFirstPersonCharacter::WallJump(const FInputActionValue& Value)
{
    UCharacterMovementComponent* MoveComponent = GetCharacterMovement();
    bool ShouldWallJump = Value.Get<bool>();
    
    if (ShouldWallJump && !IsWallJumpOnCooldown)
	{
		WallJumpRequest();
	}
}

void AFirstPersonCharacter::WallJumpRequest_Implementation()
{	
	UCharacterMovementComponent* MoveComponent = GetCharacterMovement();
	if (CanWallJump && (CurrentWallJumps < MaxWallJumps))
    {
        FVector JumpDirection = GetActorForwardVector() * 1400 + FVector(0.0f, 0.0f, MoveComponent->JumpZVelocity * 1.5);
        LaunchCharacter(JumpDirection, true, true);
        CanWallJump = false;
		CurrentWallJumps++;
        IsWallJumpOnCooldown = true;
        GetWorld()->GetTimerManager().SetTimer(WallJumpCooldownHandle, this, &AFirstPersonCharacter::EnableWallJump, WallJumpCooldownTime, false);
    }
}


void AFirstPersonCharacter::Aim(const FInputActionValue& Value)
{
	bool ShouldAim = Value.Get<bool>();

	if (ShouldAim)
	{
		if (GetCharacterMovement()->IsMovingOnGround())
		{	
			GetCharacterMovement()->MaxWalkSpeed = 400.0f;
		}
	}
	else
	{
		
		GetCharacterMovement()->MaxWalkSpeed = 900.0f;
	}
}

void AFirstPersonCharacter::StopAiming()
{
	GetCharacterMovement()->MaxWalkSpeed = 900.0f;
}

void AFirstPersonCharacter::Fire(const FInputActionValue& Value)
{
	bool Firing = Value.Get<bool>();

	if (Firing)
	{	
		ServerFireGun();
	}
}

void AFirstPersonCharacter::ServerFireGun_Implementation()
{
    if (Gun)
    {
        Gun->FireGun(); 
    }
}


void AFirstPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFirstPersonCharacter, Health);
}

float AFirstPersonCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	APlayerController* IPC = Cast<APlayerController>(EventInstigator);
	AFPSPlayerState* InstigatorPlayerState = IPC->GetPlayerState<AFPSPlayerState>();
	ServerUpdateHealth(Damage, InstigatorPlayerState);
	return Damage;
}

void AFirstPersonCharacter::ServerUpdateHealth_Implementation(float Damage, AFPSPlayerState* InstigatorPlayerState)
{
	Health = Health - Damage;
	OnRep_UpdateHealth();
	if (Health <= 0)
	{
		InstigatorPlayerState->UpdatePlayerScore();
		Destroy();
		Gun->DestroySelf();
	}
}

void AFirstPersonCharacter::OnRep_UpdateHealth()
{}

void AFirstPersonCharacter::Reload(const FInputActionValue& Value)
{
    bool Reloading = Value.Get<bool>();
	if (Reloading && Gun->ReloadAvailable)
	{
		ServerReloadGun();
	}
}

void AFirstPersonCharacter::ServerReloadGun_Implementation()
{
	Gun->Reload();
	Gun->ReloadStatus(false);
	GetWorld()->GetTimerManager().SetTimer(
		ReloadCooldownTimerHandle, 
		[this]() { Gun->ReloadStatus(true); },
		3.0f, 
		false
	);
}

void AFirstPersonCharacter::PickupItem(const FInputActionValue& Value)
{	
	bool TryPickup = Value.Get<bool>();

	if (TryPickup)
	{
		ServerPickupItem();
	}
}

void AFirstPersonCharacter::ServerPickupItem_Implementation()
{
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 500;
	FHitResult ItemHit;
	bool FoundItem = GetWorld()->LineTraceSingleByChannel(ItemHit, Start, End, ECollisionChannel::ECC_Visibility);

	if (FoundItem)
	{	
		AActor* ItemActor = ItemHit.GetActor();
		if (ItemActor)
		{
			if (ItemActor->ActorHasTag("Gun"))
			{
				Gun = Cast<ABaseGun>(ItemActor);
				Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
				Gun->SetOwner(this);
			}
		}
	}
}

void AFirstPersonCharacter::InitiateAbilityOne(const FInputActionValue& Value)
{
	bool AbilityOneUsed = Value.Get<bool>();
	if (AbilityOneUsed)
	{
		InitiatePrimaryAbility();
	}
}

void AFirstPersonCharacter::InitiatePrimaryAbility_Implementation()
{
}

void AFirstPersonCharacter::SwitchWeapon(const FInputActionValue& Value)
{
    bool SwitchWeapon = Value.Get<bool>();
    if (SwitchWeapon)
    {	
		WeaponChange();
    }
}

void AFirstPersonCharacter::WeaponChange_Implementation()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        for (int32 i = 1; i <= 9; i++)
        {	
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Step 2: Choose Gun"));
            FKey NumberKey = NumKeys[i];
            if (PC->IsInputKeyDown(NumberKey))
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Step 3: Switch Gun"));
                ABaseGun* NewGun = WeaponInventory->GetWeapon(NumberKey);
                if (NewGun) 
                {
                    Gun = NewGun;
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, TEXT("Step 4: Set Gun"));
                    Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
                    Gun->SetOwner(this);
					UpdateGunDelegate.Broadcast();
                    Gun->UpdateAmmoDelegate.Broadcast();
                }
                break;
            }
        }
    }
}

TArray<int32> AFirstPersonCharacter::GetGunData()
{
	if (Gun)
	{
		return TArray<int32>{Gun->Ammo, Gun->MaxAmmo, Gun->Magazines};
	}
	return TArray<int32>();
}

ABaseGun* AFirstPersonCharacter::GetGun()
{
	if (Gun) 
	{
		return Gun;
	}
	return nullptr;
}