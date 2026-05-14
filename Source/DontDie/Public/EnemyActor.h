// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyActor.generated.h"

UCLASS()
class DONTDIE_API AEnemyActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyActor();

	UPROPERTY(EditAnywhere)
	class UBoxComponent* BoxComp;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* StaticMeshComp;

	UPROPERTY()
	class APlayerPawn* TargetPlayer;
	
	UPROPERTY(EditAnywhere)
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere)
	float MaxHP = 50.f;

	UPROPERTY(EditAnywhere)
	float CurrentHP;

	UPROPERTY(EditAnywhere)
	float Defense = 0.f;

	UPROPERTY(EditAnywhere)
	int32 CurrencyReward = 10;

	UPROPERTY(EditAnywhere)
	float AttackPower = 10.f;

	UPROPERTY(EditAnywhere)
	int32 TraceRate = 50;

	UFUNCTION()
	void OnEnemyOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                    const FHitResult& SweepResult);

private:
	FVector dir;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
