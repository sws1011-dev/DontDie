// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurvivorActor.generated.h"

UCLASS()
class DONTDIE_API ASurvivorActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASurvivorActor();

	UPROPERTY(EditAnywhere)
	class UCapsuleComponent* CapsuleComp;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float MaxHP = 20.f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float CurrentHP;

	// 점수 보상 수치
	UPROPERTY(EditAnywhere, Category = "Rewards")
	int32 ScoreReward = 150;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UEnemyDamagedWidget> DamagedWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UEnemyHpWidget> HpWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	class UWidgetComponent* HpWidgetComp;

	void ShowDamage(float DamageAmount, FVector HitLocation);

	// 총알에 맞았을 때 좀비처럼 호출될 데미지 함수
	void TakeDamage(float DamageAmount, FVector HitLocation);

	void UpdateHpUI();

private:
	FVector MoveDirection;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
