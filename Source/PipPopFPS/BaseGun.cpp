#include "BaseGun.h"
#include "FirstPersonCharacter.h"
#include "GameFramework/PlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

ABaseGun::ABaseGun()
{
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

}

void ABaseGun::BeginPlay()
{
	Super::BeginPlay();
    Ammo = MaxAmmo;
}

void ABaseGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ABaseGun::CanFire()
{
	return (GetWorld()->GetTimeSeconds() - LastFireTime >= FireRate);
}

void ABaseGun::FireGun()
{   

        if (!Ammo || !Magazines || !CanFire()) return;
        SpawnMuzzleFlash();
        AController* OwnerController = GetOwnerController();
        if (OwnerController == nullptr) return;

        ENetRole LocalRole = GetLocalRole();
        ENetRole RemoteRole = GetRemoteRole();
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, 
            FString::Printf(TEXT("Local Role: %s, Remote Role: %s"), 
            *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), LocalRole),
            *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), RemoteRole)));
        
        // Debug if we're attempting to call the Server RPC
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Attempting ServerBulletLineTrace"));
    

        FVector Location;
        FRotator Rotation;
        OwnerController->GetPlayerViewPoint(Location, Rotation);
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("Fired"));
        ServerBulletLineTrace(OwnerController,Location, Rotation);
        LastFireTime = GetWorld()->GetTimeSeconds();
        Ammo--;
        // UE_LOG(LogTemp, Warning, TEXT("Ammo: %s"), *FString::Printf(TEXT("%d"), Ammo));
}

void ABaseGun::ServerBulletLineTrace_Implementation(AController* OwnerController, FVector Location, FRotator Rotation)
{
    FHitResult Hit;
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Attempting ServerBulletLineTrace"));
    if (HasAuthority())
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("Server Line Trace"));
        for (int32 i = 0; i < BulletsFiredPerShot; i++)
            {
                FBulletTrajectory Bullet = CalculateBulletTrajectory(Location, Rotation);
                if (GetWorld()->LineTraceSingleByChannel(Hit, Location, Bullet.End, ECollisionChannel::ECC_GameTraceChannel1))
                {
                    SpawnBulletImpact(Hit, Bullet);
                    DamageActors(OwnerController, Hit, Bullet);
                }
            }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("NO AUTHORITY"));
    }
}

void ABaseGun::DamageActors_Implementation(AController* OwnerController, FHitResult Hit, FBulletTrajectory Bullet)
{   
    if (HasAuthority())
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("Server Try Damage Actor"));
        AActor* DamagedActor = Hit.GetActor();
        if (DamagedActor && HasAuthority())
        {   
            FPointDamageEvent DamageEvent(Damage, Hit, Bullet.ShotDirection, nullptr);
            AFirstPersonCharacter* Character = Cast<AFirstPersonCharacter>(DamagedActor);
            if (Character)
            {   
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("Call Damage Character"));
                Character->TakeDamage(Damage, DamageEvent, OwnerController, this);
            }
        }
    }
    
}

FBulletTrajectory ABaseGun::CalculateBulletTrajectory(FVector Location, FRotator Rotation)
{
    FRotator SpreadRotation = Rotation;

    SpreadRotation.Pitch += FMath::RandRange(-SpreadAngles.DownAngle, SpreadAngles.UpAngle); 
    SpreadRotation.Yaw += FMath::RandRange(-SpreadAngles.LeftAngle, SpreadAngles.RightAngle);    
    FVector SpreadVector = SpreadRotation.Vector();

    FVector End = Location + SpreadVector* MaxRange; 
    FVector ShotDirection = -SpreadVector;
    return FBulletTrajectory{ ShotDirection, End };
}

AController* ABaseGun::GetOwnerController()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    return OwnerPawn ? OwnerPawn->GetController() : nullptr;
}

void ABaseGun::Reload()
{
    if (Magazines)
    {
        Ammo = MaxAmmo;
        Magazines--;
        UE_LOG(LogTemp, Warning, TEXT("Magazines: %s"), *FString::Printf(TEXT("%d"), Magazines));
    }
}

void ABaseGun::ReloadStatus(bool CanReload)
{
    ReloadAvailable = CanReload;
}

void ABaseGun::SpawnMuzzleFlash_Implementation()
{
    // Make sure the Niagara system is set up properly in the class.
    if (MuzzleFlashNiagaraSystem)
    {
        // Spawn the Niagara effect at the muzzle location
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlashNiagaraSystem, 
            Mesh, 
            TEXT("MuzzleFlash"), // Socket name to attach to
            FVector::ZeroVector, 
            FRotator::ZeroRotator, 
            EAttachLocation::SnapToTarget, 
            true, // Auto destroy when finished
            true  // Auto activate
        );
    }
}


void ABaseGun::SpawnBulletImpact_Implementation(FHitResult Hit, FBulletTrajectory Bullet)
{
    // Make sure the Niagara system for bullet impact is set up properly.
    if (BulletImpactNiagaraSystem)
    {
        // Spawn the Niagara effect at the bullet impact location
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            BulletImpactNiagaraSystem, 
            Hit.Location, 
            Bullet.ShotDirection.Rotation(),
            FVector(1.0f, 1.0f, 1.0f), // Set scale (this is optional and can be adjusted)
            true,  // Auto activate
            true   // Auto destroy when finished
        );
    }
}

void ABaseGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ABaseGun, Damage);
}