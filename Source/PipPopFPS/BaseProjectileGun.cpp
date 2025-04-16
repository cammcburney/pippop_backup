// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseProjectileGun.h"
#include "FirstPersonCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void ABaseProjectileGun::ServerBulletLineTrace(AController* OwnerController, FVector Location, FRotator Rotation)
{
    if (CanFire)
    {
        GetWorldTimerManager().ClearTimer(MissleFireTimer);
        
   
        FiredMissles = 0;
        CanFire = false;
    
        FireMissle(OwnerController, Location, Rotation);
        
        GetWorldTimerManager().SetTimer(
            MissleFireTimer,
            [this, OwnerController, Location, Rotation]() { 
                if (FiredMissles < 10)
                {
                    FireMissle(OwnerController, Location, Rotation);
                }
                else
                {
                    GetWorldTimerManager().ClearTimer(MissleFireTimer);
                    CanFire = true;
                }
            }, 
            0.1f, 
            true
        );
    }
}

void ABaseProjectileGun::FireMissle_Implementation(AController* OwnerController, FVector Location, FRotator Rotation)
{
    FiredMissles++;
    FRotator RandomConeRotation = Rotation;
    RandomConeRotation.Yaw += FMath::FRandRange(-ProjectileSpreadAngle, ProjectileSpreadAngle);
    RandomConeRotation.Pitch += FMath::FRandRange(-ProjectileSpreadAngle, ProjectileSpreadAngle);
    FVector SpawnLocation = Mesh->GetSocketLocation("BulletSpawn");
    ABaseProjectile* ProjectileFired = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, RandomConeRotation);
    if (ProjectileFired && ProjectileFired->ProjectileMovementComponent)
    {
        ProjectileFired->SetInstigatorController(OwnerController);
        USceneComponent* Target = FindTarget();
        if (Target)
        {
            ProjectileFired->ProjectileMovementComponent->HomingTargetComponent = Target;
            ProjectileFired->ProjectileMovementComponent->bIsHomingProjectile = true;
            FVector DirectionToTarget = Target->GetComponentLocation() - SpawnLocation;
            DirectionToTarget.Normalize();
            TArray<FVector> VectorDirections = {
                FVector::CrossProduct(DirectionToTarget, FVector::UpVector),
                FVector::CrossProduct(DirectionToTarget, FVector::LeftVector),
                FVector::CrossProduct(DirectionToTarget, FVector::RightVector),
                (FVector::UpVector + FVector::RightVector).GetSafeNormal(),
                (FVector::UpVector + FVector::LeftVector).GetSafeNormal()
            };
            FVector PerpDirection = VectorDirections[FMath::RandRange(0, VectorDirections.Num() - 1)];
        
            float CurveStrength = 3000.0f;
            ProjectileFired->ProjectileMovementComponent->Velocity += PerpDirection * CurveStrength;
        }
        
    }
}

USceneComponent* ABaseProjectileGun::FindTarget()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), AFirstPersonCharacter::StaticClass(), FoundActors);
    
    float ClosestDistance = 100000000.00;
    USceneComponent* TargetComponent = nullptr;
    
    AActor* OwningCharacter = GetOwner();
    
    for (AActor* TargetActor : FoundActors)
    {
        if (!TargetActor || TargetActor == GetOwner()) 
            continue;
        FVector Distance = TargetActor->GetActorLocation() - this->GetActorLocation();
        float NewDistance = Distance.Size();
        
        if (NewDistance < ClosestDistance)
        {
            USceneComponent* Component = TargetActor->GetRootComponent();   
            if (Component)
            {
                ClosestDistance = NewDistance;
                TargetComponent = Component;
                
            }
        }
    }
    return TargetComponent;
}
