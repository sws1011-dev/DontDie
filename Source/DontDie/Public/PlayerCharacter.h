// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class DONTDIE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	/* 기본 능력치 */
	// 이동 속도
	float MoveSpeed = 300.f;
	// 최대 체력
	float MaxHP = 30.f;
	// 현재 체력
	float CurrentHP = MaxHP;
	// 치명타 확률 -> 굳이 있어야될지
	float CritChance = 0.1f;
	// 발사체 개수 -> 총기로 옮기는게 좋아보임
	int32 ProjectileCount = 1;
	// 최대 목숨 개수
	int32 MaxLifeCount = 1;
	// 현재 목숨 개수
	int32 CurrentLifeCount = MaxLifeCount;

	// 재화 획득률
	float CurrencyMultiplier = 1.f;

	// 현재 장착 무기
	UPROPERTY()
	class AWeapon* CurrentWeapon;

	/* 입력 */
	UPROPERTY()
	class APlayerController* PlayerController;
	UPROPERTY(EditAnywhere)
	class UInputMappingContext* ImcPlayerInput;
	// 이동(W,A,S,D)
	UPROPERTY(EditAnywhere)
	class UInputAction* IaMove;
	// 발사(좌클릭)
	UPROPERTY(EditAnywhere)
	class UInputAction* IaFire;
	// 장전(R)
	UPROPERTY(EditAnywhere)
	class UInputAction* IaReload;

	/* UI */
	// 전투 UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TSubclassOf<class UPlayerHudWidget> PlayerHudWidgetClass;
	// 게임 오버
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TSubclassOf<class UGameOverWidget> GameOverWidgetClass;
	UPROPERTY()
	class UPlayerHudWidget* HUDWidget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* 전투 관련 함수 */
	// 목숨 개수 감소
	void DecreaseLifeCount();
	// 데미지 받음
	void OnReceiveDamage(float DamageAmount);
	// 데미지를 계산하여 적용
	void GetCalculatedDamage();

	/* UI 관련 함수 */
	// 전투 UI 새로고침
	void RefreshHUD();
	// 총알 개수 UI 새로고침
	void UpdateAmmoHUD(int32 CurrentAmmo, int32 MaxAmmo);
	// 재장전중 UI 새로고침
	void UpdateReloadingHUD(bool bIsReloading);

	/* 입력 관련 함수 */
	// 이동
	void OnInputMove(const struct FInputActionValue& value);
	// 발사
	void Fire();
	// 재장전
	void Reload();

private:
	// 이동 변수
	FVector2D MoveInput;
};
