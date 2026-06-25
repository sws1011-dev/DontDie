// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "player/BasePlayerCharacter.h"
#include "CombatPlayerCharacter.generated.h"

UCLASS()
class DONTDIE_API ACombatPlayerCharacter : public ABasePlayerCharacter
{
	GENERATED_BODY()

public:
	ACombatPlayerCharacter();

	float MoveSpeed = 300.0f;
	float MaxHP = 30.0f;
	float CurrentHP = MaxHP;
	float CritChance = 0.1f;
	int32 ProjectileCount = 1;
	int32 MaxLifeCount = 1;
	int32 CurrentLifeCount = MaxLifeCount;
	float CurrencyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class AWeapon* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping")
	class UInputMappingContext* ImcCombat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Combat")
	class UInputAction* IaFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Combat")
	class UInputAction* IaReload;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPlayerHudWidget> PlayerHudWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UGameOverWidget> GameOverWidgetClass;

	UPROPERTY()
	class UPlayerHudWidget* HUDWidget;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void DecreaseLifeCount();
	void OnReceiveDamage(float DamageAmount);
	float GetCalculatedDamage();
	void RefreshHUD();
	void UpdateAmmoHUD(int32 CurrentAmmo, int32 MaxAmmo);
	void UpdateReloadingHUD(bool bIsReloading);
	void Fire();
	void Reload();
};
