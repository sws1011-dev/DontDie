// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildableActor.h"

#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ABuildableActor::ABuildableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	SetRootComponent(SceneRootComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneRootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ValidPreviewMaterialFinder(TEXT("/Game/Build/Materials/M_BuildPreview_Valid.M_BuildPreview_Valid"));
	if (ValidPreviewMaterialFinder.Succeeded())
	{
		ValidPreviewMaterial = ValidPreviewMaterialFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> InvalidPreviewMaterialFinder(TEXT("/Game/Build/Materials/M_BuildPreview_Invalid.M_BuildPreview_Invalid"));
	if (InvalidPreviewMaterialFinder.Succeeded())
	{
		InvalidPreviewMaterial = InvalidPreviewMaterialFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HoverPreviewMaterialFinder(TEXT("/Game/Build/Materials/M_BuildPreview_Hover.M_BuildPreview_Hover"));
	if (HoverPreviewMaterialFinder.Succeeded())
	{
		HoverPreviewMaterial = HoverPreviewMaterialFinder.Object;
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultHoverPreviewMaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (DefaultHoverPreviewMaterialFinder.Succeeded())
		{
			HoverPreviewMaterial = DefaultHoverPreviewMaterialFinder.Object;
		}
	}
}

void ABuildableActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateStaticMeshSize();
}

void ABuildableActor::SetPreviewMode(bool bEnabled)
{
	bPreviewMode = bEnabled;
	SetActorCollisionEnabled(!bPreviewMode);

	if (bPreviewMode)
	{
		CacheOriginalMaterials();
		SetPlacementValid(true);
	}
	else
	{
		RestoreOriginalMaterials();
	}
}

void ABuildableActor::SetPlacementValid(bool bIsValid)
{
	if (!bPreviewMode)
	{
		return;
	}

	ApplyMaterial(bIsValid ? ValidPreviewMaterial : InvalidPreviewMaterial);
}

void ABuildableActor::SetPreviewHighlighted(bool bHighlighted)
{
	if (!bPreviewMode)
	{
		return;
	}

	if (bHighlighted)
	{
		ApplyMaterial(GetHoverPreviewMaterial());
	}
	else
	{
		SetPlacementValid(true);
	}
}

void ABuildableActor::SetDemolitionPreview(bool bEnabled)
{
	if (bPreviewMode || bDemolitionPreview == bEnabled)
	{
		return;
	}

	bDemolitionPreview = bEnabled;
	if (bDemolitionPreview)
	{
		CacheOriginalMaterials();
		ApplyMaterial(InvalidPreviewMaterial);
	}
	else
	{
		RestoreOriginalMaterials();
	}
}

void ABuildableActor::SetHoverPreview(bool bEnabled)
{
	if (bPreviewMode || bDemolitionPreview || bHoverPreview == bEnabled)
	{
		return;
	}

	bHoverPreview = bEnabled;
	if (bHoverPreview)
	{
		CacheOriginalMaterials();
		ApplyMaterial(GetHoverPreviewMaterial());
	}
	else
	{
		RestoreOriginalMaterials();
	}
}

void ABuildableActor::FinalizeBuild()
{
	SetPreviewMode(false);
}

bool ABuildableActor::IsPreviewMode() const
{
	return bPreviewMode;
}

void ABuildableActor::SetBuildSize(const FVector& NewBuildSize)
{
	FootprintSize = FVector2D(
		FMath::Max(NewBuildSize.X, 1.0f),
		FMath::Max(NewBuildSize.Y, 1.0f));
	ActorHalfHeight = FMath::Max(NewBuildSize.Z * 0.5f, 0.0f);
	PlacementBoxHalfHeight = FMath::Max(NewBuildSize.Z * 0.5f, 1.0f);

	UpdateStaticMeshSize();
}

void ABuildableActor::SetBuildMesh(UStaticMesh* NewMesh)
{
	if (StaticMeshComponent == nullptr || NewMesh == nullptr)
	{
		return;
	}

	StaticMeshComponent->SetStaticMesh(NewMesh);
	OriginalMaterials.Empty();
	UpdateStaticMeshSize();
}

void ABuildableActor::SetBuildingData(FName NewBuildingRowName, FName NewBuildingTypeID, int32 NewTier)
{
	BuildingRowName = NewBuildingRowName;
	BuildingTypeID = NewBuildingTypeID;
	Tier = FMath::Max(NewTier, 1);
}

void ABuildableActor::CacheOriginalMaterials()
{
	if (!OriginalMaterials.IsEmpty())
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent == nullptr)
		{
			continue;
		}

		TArray<UMaterialInterface*> Materials;
		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			Materials.Add(MeshComponent->GetMaterial(MaterialIndex));
		}

		OriginalMaterials.Add(MeshComponent, Materials);
	}
}

void ABuildableActor::RestoreOriginalMaterials()
{
	for (const TPair<UMeshComponent*, TArray<UMaterialInterface*>>& OriginalMaterialPair : OriginalMaterials)
	{
		UMeshComponent* MeshComponent = OriginalMaterialPair.Key;
		if (MeshComponent == nullptr)
		{
			continue;
		}

		const TArray<UMaterialInterface*>& Materials = OriginalMaterialPair.Value;
		for (int32 MaterialIndex = 0; MaterialIndex < Materials.Num(); ++MaterialIndex)
		{
			MeshComponent->SetMaterial(MaterialIndex, Materials[MaterialIndex]);
		}
	}
}

void ABuildableActor::SetActorCollisionEnabled(bool bEnabled)
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
}

UMaterialInterface* ABuildableActor::GetHoverPreviewMaterial()
{
	if (HoverPreviewMaterial != nullptr)
	{
		return HoverPreviewMaterial;
	}

	if (RuntimeHoverPreviewMaterial == nullptr && ValidPreviewMaterial != nullptr)
	{
		RuntimeHoverPreviewMaterial = UMaterialInstanceDynamic::Create(ValidPreviewMaterial, this);
		if (RuntimeHoverPreviewMaterial != nullptr)
		{
			const FLinearColor HoverColor = FLinearColor::White;
			RuntimeHoverPreviewMaterial->SetVectorParameterValue(TEXT("Color"), HoverColor);
			RuntimeHoverPreviewMaterial->SetVectorParameterValue(TEXT("BaseColor"), HoverColor);
			RuntimeHoverPreviewMaterial->SetVectorParameterValue(TEXT("Tint"), HoverColor);
			RuntimeHoverPreviewMaterial->SetVectorParameterValue(TEXT("PreviewColor"), HoverColor);
		}
	}

	return RuntimeHoverPreviewMaterial != nullptr ? RuntimeHoverPreviewMaterial : ValidPreviewMaterial;
}

void ABuildableActor::ApplyMaterial(UMaterialInterface* Material)
{
	if (Material == nullptr)
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent == nullptr)
		{
			continue;
		}

		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			MeshComponent->SetMaterial(MaterialIndex, Material);
		}
	}
}

void ABuildableActor::UpdateStaticMeshSize()
{
	if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr)
	{
		return;
	}

	const FBoxSphereBounds MeshBounds = StaticMeshComponent->GetStaticMesh()->GetBounds();
	const FVector MeshSize = MeshBounds.BoxExtent * 2.0f;

	if (MeshSize.X <= KINDA_SMALL_NUMBER || MeshSize.Y <= KINDA_SMALL_NUMBER || MeshSize.Z <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector NewScale(
		FMath::Max(FootprintSize.X, 1.0f) / MeshSize.X,
		FMath::Max(FootprintSize.Y, 1.0f) / MeshSize.Y,
		FMath::Max(ActorHalfHeight * 2.0f, 1.0f) / MeshSize.Z);

	StaticMeshComponent->SetRelativeScale3D(NewScale);
	StaticMeshComponent->SetRelativeLocation(-MeshBounds.Origin * NewScale);
}
