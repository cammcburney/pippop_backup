// Fill out your copyright notice in the Description page of Project Settings.


#include "SneakyCatCharacter.h"

void ASneakyCatCharacter::InitiatePrimaryAbility_Implementation()
{
    if (UPrimitiveComponent* PrimitiveComponent = this->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimitiveComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    }
    if (HiddenMaterial && GetMesh())
    {
        MI = UMaterialInstanceDynamic::Create(HiddenMaterial, this);
        if (MI)
        {
            GetMesh()->SetMaterial(0, MI);
        }
    }
    
    GetWorld()->GetTimerManager().SetTimer(
        PrimaryAbilityCooldownHandle, 
        this, 
        &ASneakyCatCharacter::EndSneakAbility,
        10.0f,
        false
    );
}

void ASneakyCatCharacter::EndSneakAbility()
{
    if (UPrimitiveComponent* PrimitiveComponent = this->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimitiveComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    }
    GetMesh()->SetMaterial(0, DefaultMaterial);
}