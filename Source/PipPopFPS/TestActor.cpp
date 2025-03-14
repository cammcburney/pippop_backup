// Fill out your copyright notice in the Description page of Project Settings.


#include "TestActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
ATestActor::ATestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ReplicatedVar = 100.0f;
	bReplicates = true;
}

// Called when the game starts or when spawned
void ATestActor::BeginPlay()
{
	Super::BeginPlay();
	
	// SetReplicates(true);
	SetReplicateMovement(true);

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TestTimer, this, &ATestActor::DecreaseReplicatedVar, 2.0f, false);
	}

	NetMulticastRPCFunction();

}

// Called every frame
void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// if (HasAuthority())
	// {
	// 	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Server"));
	// }
	// else
	// {
	// 	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Client"));
	// }
}

void ATestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATestActor, ReplicatedVar);
}

void ATestActor::OnRep_ReplicatedVar()
{
	if(HasAuthority())
	{	
		FVector NewLocation = GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);
		SetActorLocation(NewLocation);
		// GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Server"));
	}
	// else
	// {
	// 	// GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Client %d: OnRep_ReplicatedVar"), static_cast<int>(GPlayInEditorID)));
	// }
}

void ATestActor::DecreaseReplicatedVar()
{
	if (HasAuthority())
	{
		ReplicatedVar -= 1.0f;
		OnRep_ReplicatedVar();
		if (ReplicatedVar > 0)
		{
			GetWorld()->GetTimerManager().SetTimer(TestTimer, this, &ATestActor::DecreaseReplicatedVar, 2.0f, false);
		}
	}
}

void ATestActor::NetMulticastRPCFunction_Implementation()
{
	if (HasAuthority())
	{
		// GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Server")));
		GetWorld()->GetTimerManager().SetTimer(TestTimer, this, &ATestActor::NetMulticastRPCFunction, 2.0f, false);
	}
	else
	{
		// GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Client")));
	}

	if (!IsRunningDedicatedServer())
	{	
		FVector SpawnLocation = GetActorLocation() + GetActorUpVector() * 100.0f;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, SpawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	}
}
