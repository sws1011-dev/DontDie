// Fill out your copyright notice in the Description page of Project Settings.


#include "CampGridTile.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
ACampGridTile::ACampGridTile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(SceneRoot);
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		TileMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

// Called when the game starts or when spawned
void ACampGridTile::BeginPlay()
{
	Super::BeginPlay();

	if (TileMesh != nullptr)
	{
		DynamicMaterial = TileMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	ApplyStateColor();
}

// Called every frame
void ACampGridTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACampGridTile::SetTileState(ECampGridTileState NewState)
{
	TileState = NewState;
	ApplyStateColor();
}

void ACampGridTile::SetGridIndex(int32 InX, int32 InY)
{
	GridX = InX;
	GridY = InY;
}

void ACampGridTile::SetTileSize(float InTileSize)
{
	if (TileMesh == nullptr)
	{
		return;
	}

	const float PlaneSize = 100.0f;
	const float Scale = InTileSize / PlaneSize;
	TileMesh->SetRelativeScale3D(FVector(Scale, Scale, 1.0f));
}

void ACampGridTile::ApplyStateColor()
{
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	FLinearColor TargetColor = NormalColor;
	if (TileState == ECampGridTileState::Selected)
	{
		TargetColor = SelectedColor;
	}
	else if (TileState == ECampGridTileState::Blocked)
	{
		TargetColor = BlockedColor;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), TargetColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TargetColor);
}

