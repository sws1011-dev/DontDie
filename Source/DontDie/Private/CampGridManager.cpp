// Fill out your copyright notice in the Description page of Project Settings.


#include "CampGridManager.h"

#include "CampGridTile.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"


// Sets default values
ACampGridManager::ACampGridManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

// Called when the game starts or when spawned
void ACampGridManager::BeginPlay()
{
	Super::BeginPlay();

	SpawnGridTiles();
}

// Called every frame
void ACampGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACampGridManager::SpawnGridTiles()
{
	if (TileVisualClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("CampGridManager: TileVisualClass is not assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	SpawnedTiles.Reset();

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			const FVector TileLocation = GetTileWorldLocation(X, Y);
			const FRotator TileRotation = GetActorRotation();

			AActor* Tile = World->SpawnActor<AActor>(TileVisualClass, TileLocation, TileRotation);
			if (Tile != nullptr)
			{
				Tile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

				if (ACampGridTile* GridTile = Cast<ACampGridTile>(Tile))
				{
					GridTile->SetGridIndex(X, Y);
					GridTile->SetTileSize(TileSize);
				}

				SpawnedTiles.Add(Tile);
			}
		}
	}
}

FVector ACampGridManager::GetTileWorldLocation(int32 X, int32 Y) const
{
	const float OffsetX = (static_cast<float>(X) - (static_cast<float>(GridWidth) - 1.0f) * 0.5f) * TileSize;
	const float OffsetY = (static_cast<float>(Y) - (static_cast<float>(GridHeight) - 1.0f) * 0.5f) * TileSize;

	return GetActorLocation() + GetActorForwardVector() * OffsetX + GetActorRightVector() * OffsetY;
}

