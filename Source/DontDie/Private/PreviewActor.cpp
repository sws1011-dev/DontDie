// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
APreviewActor::APreviewActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(SceneRoot);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PreviewMesh->SetStaticMesh(CubeMesh.Object);
	}
}

// Called when the game starts or when spawned
void APreviewActor::BeginPlay()
{
	Super::BeginPlay();

	if (PreviewMesh != nullptr)
	{
		DynamicMaterial = PreviewMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	SetPreviewValid(true);
}

// Called every frame
void APreviewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APreviewActor::SetPreviewValid(bool bIsValid)
{
	ApplyPreviewColor(bIsValid ? ValidColor : InvalidColor);
}

void APreviewActor::SetPreviewSize(float TileSize)
{
	SetPreviewSize2D(FVector2D(TileSize, TileSize), 50.0f);
}

void APreviewActor::SetPreviewSize2D(const FVector2D& FootprintSize, float HalfHeight)
{
	if (PreviewMesh == nullptr)
	{
		return;
	}

	const float CubeSize = 100.0f;
	const FVector SafeSize(
		FMath::Max(FootprintSize.X, 1.0f),
		FMath::Max(FootprintSize.Y, 1.0f),
		FMath::Max(HalfHeight * 2.0f, 1.0f));

	PreviewMesh->SetRelativeScale3D(FVector(
		SafeSize.X / CubeSize,
		SafeSize.Y / CubeSize,
		SafeSize.Z / CubeSize));
}

void APreviewActor::ApplyPreviewColor(const FLinearColor& Color)
{
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

