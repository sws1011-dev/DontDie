// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class DONTDIE_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeapon();

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* WeaponMeshComp;

	UPROPERTY(EditAnywhere)
	class UArrowComponent* FirePosition;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABullet> BulletFactory;

	UPROPERTY(EditAnywhere, Category="Weapon|Stats")
	float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere, Category="Weapon|Stats")
	float FireRate = 3.f;

	UPROPERTY(EditAnywhere, Category="Weapon|Stats")
	int32 MaxAmmo = 12;

	UPROPERTY(EditAnywhere, Category="Weapon|Stats")
	float ReloadSpeed = 2.0f;

	int32 CurrentAmmo;
	bool bIsReloading = false;

	void Fire(float DamageMultiplier = 1.0f);
	void Reload();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	float LastFireTime = 0.f;

	struct FTimerHandle ReloadTimerHandle;

	void OnReloadComplete();

	void UpdatePlayerHUD();
};
