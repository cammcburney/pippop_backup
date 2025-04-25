// Fill out your copyright notice in the Description page of Project Settings.


#include "TestActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ATestActor::ATestActor()
{

	PrimaryActorTick.bCanEverTick = true;
	ReplicatedVar = 100.0f;
	bReplicates = true;
}


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
	}

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
		GetWorld()->GetTimerManager().SetTimer(TestTimer, this, &ATestActor::NetMulticastRPCFunction, 2.0f, false);
	}

	if (!IsRunningDedicatedServer())
	{	
		FVector SpawnLocation = GetActorLocation() + GetActorUpVector() * 100.0f;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, SpawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	}
}
