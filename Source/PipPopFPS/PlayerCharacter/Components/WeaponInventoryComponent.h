#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Weapons/HitScan/BaseGun.h"
#include "WeaponInventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PIPPOPFPS_API UWeaponInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UWeaponInventoryComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons")
	TMap<FKey, TSubclassOf<ABaseGun>> PlayerWeapons;

	void SetWeapon(FKey AccessKey, TSubclassOf<ABaseGun> PlayerWeaponClass);

	ABaseGun* GetWeapon(FKey AccessKey);
	
protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TMap<FKey, ABaseGun*> SpawnedWeapons;
};