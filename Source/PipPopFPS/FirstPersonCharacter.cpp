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
#include "BaseGun.h"
#include "DrawDebugHelpers.h"  // Include for drawing debug lines

AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bUseControllerRotationYaw = true;
	IsWallSliding = false;
	CanWallJump = false;
	WallJumpSlideEnabled = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FirstPersonCamera->bUsePawnControlRotation = false;
}

void AFirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);

	Health = MaxHealth;

	Gun = GetWorld()->SpawnActor<ABaseGun>(GunClass);
	if (Gun)
	{	
		Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
		Gun->SetOwner(this);
	}
}

void AFirstPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UCharacterMovementComponent* MoveComponent = GetCharacterMovement();

	if (MoveComponent->IsMovingOnGround())
	{
		IsWallSliding = false;
	}
	if (!IsWallSliding)
	{
		WallSliding();
	}
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

void AFirstPersonCharacter::WallSliding()
{
    UCharacterMovementComponent* MoveComponent = GetCharacterMovement();

    if (MoveComponent->IsFalling())
    {
        FVector Start = GetActorLocation();
        FVector End = Start + GetActorForwardVector() * 300;

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

                IsWallSliding = true;
                CanWallJump = true;
                WallJumpSlideEnabled = true;
            }
        }
    }
}


void AFirstPersonCharacter::EnableWallJump()
{
	WallJumpSlideEnabled = true;
}

void AFirstPersonCharacter::WallJump(const FInputActionValue& Value)
{
    UCharacterMovementComponent* MoveComponent = GetCharacterMovement();
    bool ShouldWallJump = Value.Get<bool>();
    
    if (ShouldWallJump && CanWallJump && IsWallSliding && WallJumpSlideEnabled)
    {
        FVector JumpDirection = GetActorForwardVector() * 1500 + FVector(0.0f, 0.0f, 400.0f);
        LaunchCharacter(JumpDirection, true, true);

        CanWallJump = false;
        WallJumpSlideEnabled = false;
        IsWallSliding = false;
        
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
		if (Gun)
		{
			Gun->FireGun();
		}
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
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Call Damage Actors"));
	ServerUpdateHealth(Damage);
	return Damage;
}

void AFirstPersonCharacter::ServerUpdateHealth_Implementation(float Damage)
{
	if (HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("Server call update hp"));
		Health = Health - Damage;
		OnRep_UpdateHealth();
	}
}

void AFirstPersonCharacter::OnRep_UpdateHealth()
{
	FString HealthMessage = FString::Printf(TEXT("Update REP health: %f"), Health);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, *HealthMessage);
}

void AFirstPersonCharacter::Reload(const FInputActionValue& Value)
{
    bool Reloading = Value.Get<bool>();

    if (Reloading && Gun->ReloadAvailable)
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
}

void AFirstPersonCharacter::PickupItem(const FInputActionValue& Value)
{	
	bool TryPickup = Value.Get<bool>();

	if (TryPickup)
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
}
