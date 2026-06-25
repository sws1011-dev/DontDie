// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildableActor.h"

#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"

ABuildableActor::ABuildableActor()
{
	PrimaryActorTick.bCanEverTick = false;
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
		if (BuiltMaterial != nullptr)
		{
			ApplyMaterial(BuiltMaterial);
		}
		else
		{
			RestoreOriginalMaterials();
		}
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

void ABuildableActor::FinalizeBuild()
{
	SetPreviewMode(false);
}

bool ABuildableActor::IsPreviewMode() const
{
	return bPreviewMode;
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
