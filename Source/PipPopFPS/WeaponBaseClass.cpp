// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseClass.h"

AWeaponBaseClass::AWeaponBaseClass()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
}


void AWeaponBaseClass::BeginPlay()
{
	Super::BeginPlay();
}


void AWeaponBaseClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

