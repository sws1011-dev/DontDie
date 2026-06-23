// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampGridTile.generated.h"

UENUM(BlueprintType)
enum class ECampGridTileState : uint8
{
	Normal,
	Selected,
	Blocked
};

UCLASS()
class DONTDIE_API ACampGridTile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACampGridTile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	class USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	class UStaticMeshComponent* TileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile")
	FLinearColor NormalColor = FLinearColor(0.0f, 0.7f, 0.3f, 0.25f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile")
	FLinearColor SelectedColor = FLinearColor(0.0f, 1.0f, 0.1f, 0.6f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile")
	FLinearColor BlockedColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.45f);

	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetTileState(ECampGridTileState NewState);

	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetGridIndex(int32 InX, int32 InY);

	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetTileSize(float InTileSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;

	ECampGridTileState TileState = ECampGridTileState::Normal;
	int32 GridX = 0;
	int32 GridY = 0;

	void ApplyStateColor();
};
