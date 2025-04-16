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

void ABaseGun::FireGun_Implementation()
{   

        if (!Ammo || !Magazines || !CanFire()) return;
        SpawnMuzzleFlash();
        AController* OwnerController = GetOwnerController();
        if (OwnerController == nullptr) return;
        FVector Location;
        FRotator Rotation;
        OwnerController->GetPlayerViewPoint(Location, Rotation);
        ServerBulletLineTrace(OwnerController, Location, Rotation);
        LastFireTime = GetWorld()->GetTimeSeconds();
        Ammo--;
        UE_LOG(LogTemp, Warning, TEXT("Ammo: %s"), *FString::Printf(TEXT("%d"), Ammo));
}

void ABaseGun::ServerBulletLineTrace_Implementation(AController* OwnerController, FVector Location, FRotator Rotation)
{
    FHitResult Hit;
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

void ABaseGun::DamageActors_Implementation(AController* OwnerController, FHitResult Hit, FBulletTrajectory Bullet)
{   
    if (HasAuthority())
    {
        AActor* DamagedActor = Hit.GetActor();
        if (DamagedActor && HasAuthority())
        {   
            float DamageReduction = MaxRange - Hit.Distance;
            if (DamageReduction < 0)
            {
                Damage = Damage - DamageReduction;
            }
            FPointDamageEvent DamageEvent(Damage, Hit, Bullet.ShotDirection, nullptr);
            AFirstPersonCharacter* Character = Cast<AFirstPersonCharacter>(DamagedActor);
            UE_LOG(LogTemp, Warning, TEXT("Damage Calculation Debug"));
            UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *DamagedActor->GetName());
            UE_LOG(LogTemp, Warning, TEXT("Base Damage: %.2f"), Damage);
            UE_LOG(LogTemp, Warning, TEXT("Max Range: %.2f"), MaxRange);
            UE_LOG(LogTemp, Warning, TEXT("Hit Distance: %.2f"), Hit.Distance);
            if (Character)
            {   
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

void ABaseGun::Reload_Implementation()
{
    if (Magazines && ReloadAvailable)
    {
        Ammo = MaxAmmo;
        Magazines--;
        UE_LOG(LogTemp, Warning, TEXT("Magazines: %s"), *FString::Printf(TEXT("%d"), Magazines));
    }
}

void ABaseGun::ReloadStatus_Implementation(bool CanReload)
{
    ReloadAvailable = CanReload;
    FString DebugMessage = FString::Printf(TEXT("CanReload: %s"), ReloadAvailable ? TEXT("True") : TEXT("False"));
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, DebugMessage);

}

void ABaseGun::SpawnMuzzleFlash_Implementation()
{
    if (MuzzleFlashNiagaraSystem)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlashNiagaraSystem, 
            Mesh, 
            TEXT("MuzzleFlash"),
            FVector::ZeroVector, 
            FRotator::ZeroRotator, 
            EAttachLocation::SnapToTarget, 
            true, 
            true  
        );
    }
}


void ABaseGun::SpawnBulletImpact_Implementation(FHitResult Hit, FBulletTrajectory Bullet)
{
    if (BulletImpactNiagaraSystem)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            BulletImpactNiagaraSystem, 
            Hit.Location, 
            Bullet.End.Rotation(),
            FVector(1.0f, 1.0f, 1.0f),
            true,
            true 
        );
    }
}

void ABaseGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ABaseGun, Damage);
}