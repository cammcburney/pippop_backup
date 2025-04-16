#include "BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"  
#include "Components/StaticMeshComponent.h" 
#include "FirstPersonCharacter.h"
#include "Engine/DamageEvents.h"
#include "UObject/ConstructorHelpers.h"

ABaseProjectile::ABaseProjectile() 
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;
    
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    CollisionComponent->InitSphereRadius(20.0f);
    
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->SetNotifyRigidBodyCollision(true);
    CollisionComponent->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);

    RootComponent = CollisionComponent;
    
    ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
    ProjectileMeshComponent->SetupAttachment(RootComponent);
    ProjectileMeshComponent->SetStaticMesh(ProjectileMesh);
    ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMeshComponent->SetVisibility(true);
    ProjectileMeshComponent->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f)); 

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
    ProjectileMovementComponent->InitialSpeed = 200.0f; 
    ProjectileMovementComponent->MaxSpeed = 2000.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
    ProjectileMovementComponent->ProjectileGravityScale = 0.3f;
    ProjectileMovementComponent->bIsHomingProjectile = false;
    ProjectileMovementComponent->HomingAccelerationMagnitude = 5000.0f;
    
    InitialLifeSpan = 10.0f;
}

void ABaseProjectile::BeginPlay()
{
    Super::BeginPlay();
    SetReplicateMovement(true);
}


void ABaseProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector Velocity = ProjectileMovementComponent->Velocity;

    if (!Velocity.IsZero())
    {
        FRotator NewRotation = Velocity.Rotation();
        ProjectileMeshComponent->SetWorldRotation(NewRotation);
    }
}

void ABaseProjectile::SetInstigatorController(AController* Controller)
{
    InstigatorController = Controller;
}


void ABaseProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (HasAuthority())
    {
        if (OtherActor != GetOwner())
        {
            FPointDamageEvent DamageEvent(Damage, Hit, -GetActorRotation().Vector(), nullptr);
            AFirstPersonCharacter* Character = Cast<AFirstPersonCharacter>(OtherActor);
            if (Character)
            {   
                Character->TakeDamage(Damage, DamageEvent, InstigatorController, this);
            }
            Destroy();
        }
    }
}