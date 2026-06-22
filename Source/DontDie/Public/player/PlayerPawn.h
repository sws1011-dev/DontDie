// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

UCLASS()
class DONTDIE_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

	UPROPERTY(EditAnywhere)
	class UCapsuleComponent* CapsuleComp;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(EditAnywhere, Category="Weapon")
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class AWeapon* CurrentWeapon;

	UPROPERTY(EditAnywhere)
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere)
	float MaxHP = 30.f;

	UPROPERTY(EditAnywhere)
	float CurrentHP;

	UPROPERTY(EditAnywhere)
	float CritChance = 0.1f;

	UPROPERTY(EditAnywhere)
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere)
	int32 LifeCount = 1;

	UPROPERTY(EditAnywhere)
	float CurrencyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere)
	class UInputMappingContext* ImcPlayerInput;

	UPROPERTY(EditAnywhere)
	class UInputAction* IaMove;

	UPROPERTY(EditAnywhere)
	class UInputAction* IaFire;

	UPROPERTY(EditAnywhere)
	class UInputAction* IaReload;

	UPROPERTY(EditAnywhere)
	class APlayerController* PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TSubclassOf<class UPlayerHudWidget> HUDClass;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<class UUserWidget> GameOverWidgetClass;

	UPROPERTY()
	class UPlayerHudWidget* HUDWidget;

	void DecreaseLife();
	void OnReceiveDamage(float DamageAmount);
	
	int32 CurrentLife = 1;

	UFUNCTION(BlueprintCallable)
	float GetCalculatedDamage();

	void Reload();
	void RefreshHUD();
	void UpdateAmmoHUD(int32 CurrentAmmo, int32 MaxAmmo);
	void UpdateReloadingHUD(bool bIsReloading);

private:
	FVector2D MoveInput;

	void OnInputMove(const struct FInputActionValue& value);
	void Fire();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
