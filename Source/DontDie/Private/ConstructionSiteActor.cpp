// Fill out your copyright notice in the Description page of Project Settings.

#include "ConstructionSiteActor.h"

#include "Components/BoxComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AConstructionSiteActor::AConstructionSiteActor()
{
	InteractionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBoxComponent"));
	InteractionBoxComponent->SetupAttachment(GetRootComponent());
	InteractionBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBoxComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionBoxComponent->SetGenerateOverlapEvents(false);
	InteractionBoxComponent->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ConstructionSiteMaterialFinder(TEXT("/Game/Build/Materials/M_BuildPreview_Hover.M_BuildPreview_Hover"));
	if (ConstructionSiteMaterialFinder.Succeeded())
	{
		ConstructionSiteMaterial = ConstructionSiteMaterialFinder.Object;
	}
}

void AConstructionSiteActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateConstructionCollision();
	ApplyConstructionMaterial();
}

void AConstructionSiteActor::InitializeConstructionSite(
	FName InBuildingRowName,
	FName InBuildingTypeID,
	int32 InTier,
	const FVector& InBuildSize,
	TSubclassOf<ABuildableActor> InTargetBuildActorClass,
	UStaticMesh* InStaticMesh)
{
	SetBuildingData(InBuildingRowName, InBuildingTypeID, InTier);
	SetBuildSize(InBuildSize);
	SetBuildMesh(InStaticMesh);
	TargetBuildActorClass = InTargetBuildActorClass;
	UpdateConstructionCollision();
	ApplyConstructionMaterial();
}

ABuildableActor* AConstructionSiteActor::CompleteConstruction()
{
	if (TargetBuildActorClass == nullptr)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABuildableActor* CompletedActor = World->SpawnActor<ABuildableActor>(TargetBuildActorClass, GetActorTransform(), SpawnParams);
	if (CompletedActor == nullptr)
	{
		return nullptr;
	}

	CompletedActor->SetBuildingData(BuildingRowName, BuildingTypeID, Tier);
	CompletedActor->SetBuildSize(FVector(FootprintSize.X, FootprintSize.Y, ActorHalfHeight * 2.0f));
	if (StaticMeshComponent != nullptr)
	{
		CompletedActor->SetBuildMesh(StaticMeshComponent->GetStaticMesh());
	}
	CompletedActor->FinalizeBuild();

	Destroy();
	return CompletedActor;
}

void AConstructionSiteActor::UpdateConstructionCollision()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr || PrimitiveComponent == InteractionBoxComponent)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetGenerateOverlapEvents(false);
		PrimitiveComponent->SetCanEverAffectNavigation(false);
	}

	if (InteractionBoxComponent != nullptr)
	{
		InteractionBoxComponent->SetBoxExtent(FVector(
			FMath::Max(FootprintSize.X * 0.5f, 30.0f),
			FMath::Max(FootprintSize.Y * 0.5f, 30.0f),
			FMath::Max(ActorHalfHeight, 50.0f)));
		InteractionBoxComponent->SetRelativeLocation(FVector(0.0f, 0.0f, ActorHalfHeight));
	}
}

void AConstructionSiteActor::ApplyConstructionMaterial()
{
	if (ConstructionSiteMaterial == nullptr)
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
			MeshComponent->SetMaterial(MaterialIndex, ConstructionSiteMaterial);
		}
	}
}
