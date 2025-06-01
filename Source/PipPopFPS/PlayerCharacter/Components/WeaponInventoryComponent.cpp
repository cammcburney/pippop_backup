
#include "WeaponInventoryComponent.h"

UWeaponInventoryComponent::UWeaponInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UWeaponInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponInventoryComponent::SetWeapon(FKey AccessKey, TSubclassOf<ABaseGun> PlayerWeaponClass)
{
	PlayerWeapons.Add(AccessKey, PlayerWeaponClass);
}

ABaseGun* UWeaponInventoryComponent::GetWeapon(FKey AccessKey)
{
	if (TSubclassOf<ABaseGun> GunClass = PlayerWeapons[AccessKey])
	{
		if (SpawnedWeapons.Contains(AccessKey))
		{
			return SpawnedWeapons[AccessKey];
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			ABaseGun* NewGun = GetWorld()->SpawnActor<ABaseGun>(GunClass, SpawnParams);
			if (NewGun)
			{
				SpawnedWeapons.Add(AccessKey, NewGun);
				return NewGun;
			}
		}
	}
	
	return nullptr;
}