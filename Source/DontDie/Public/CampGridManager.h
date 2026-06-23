// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampGridManager.generated.h"

UCLASS()
class DONTDIE_API ACampGridManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACampGridManager();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	class USceneComponent* SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1"))
	int32 GridWidth = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1"))
	int32 GridHeight = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1.0"))
	float TileSize = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	TSubclassOf<AActor> TileVisualClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TArray<AActor*> SpawnedTiles;

	void SpawnGridTiles();
	FVector GetTileWorldLocation(int32 X, int32 Y) const;
};
