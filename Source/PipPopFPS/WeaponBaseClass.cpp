// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseClass.h"

// Sets default values
AWeaponBaseClass::AWeaponBaseClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	
}

// Called when the game starts or when spawned
void AWeaponBaseClass::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBaseClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

