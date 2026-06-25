// Fill out your copyright notice in the Description page of Project Settings.


#include "CampBuildComponent.h"

#include "BuildingDataRow.h"
#include "BuildableActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/DataTable.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UCampBuildComponent::UCampBuildComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCampBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBuildMode)
	{
		UpdateBuildTrace();
	}
}

void UCampBuildComponent::ToggleBuildMode()
{
	SetBuildMode(!bBuildMode);
}

void UCampBuildComponent::SetBuildMode(bool bEnabled)
{
	bBuildMode = bEnabled;

	if (!bBuildMode)
	{
		bHasCurrentTraceHit = false;
		bCanPlaceCurrentPreview = false;
		bSurfacePlacementBlocked = false;
		DestroyBuildPreviewActor();
	}
	else
	{
		LoadBuildingDataRows();
		if (BuildingRowNames.IsValidIndex(SelectedBuildingIndex))
		{
			SelectBuildingByIndex(SelectedBuildingIndex);
		}
		else if (!BuildingRowNames.IsEmpty())
		{
			SelectBuildingByIndex(0);
		}

		SpawnBuildPreviewActor();
	}
}

bool UCampBuildComponent::IsBuildModeEnabled() const
{
	return bBuildMode;
}

bool UCampBuildComponent::HasCurrentTraceHit() const
{
	return bHasCurrentTraceHit;
}

FVector UCampBuildComponent::GetCurrentTraceHitLocation() const
{
	return bHasCurrentTraceHit ? CurrentTraceHit.ImpactPoint : FVector::ZeroVector;
}

FIntPoint UCampBuildComponent::GetCurrentGridIndex() const
{
	return CurrentGridIndex;
}

bool UCampBuildComponent::CanPlaceCurrentPreview() const
{
	return bCanPlaceCurrentPreview;
}

bool UCampBuildComponent::ConfirmBuild()
{
	if (!bBuildMode)
	{
		return false;
	}

	if (!bHasCurrentTraceHit)
	{
		return false;
	}

	if (!bCanPlaceCurrentPreview)
	{
		return false;
	}

	if (BuildActorClass == nullptr)
	{
		return false;
	}

	if (CurrentPreviewActor == nullptr)
	{
		return false;
	}

	ABuildableActor* BuiltActor = CurrentPreviewActor;
	CurrentPreviewActor = nullptr;
	BuiltActor->FinalizeBuild();

	SpawnBuildPreviewActor();
	UpdateBuildPreviewActor();

	return true;
}

void UCampBuildComponent::CancelBuild()
{
	SetBuildMode(false);
}

void UCampBuildComponent::RotatePreview(float YawDelta)
{
	CurrentBuildYaw = FMath::Fmod(CurrentBuildYaw + YawDelta, 360.0f);
	if (CurrentBuildYaw < 0.0f)
	{
		CurrentBuildYaw += 360.0f;
	}

	UpdateBuildPreviewActor();
}

void UCampBuildComponent::ChangeSelectedBuilding(int32 Direction)
{
	if (Direction == 0)
	{
		return;
	}

	if (BuildingRowNames.IsEmpty())
	{
		LoadBuildingDataRows();
	}

	if (BuildingTypeIDs.IsEmpty())
	{
		return;
	}

	if (BuildingTypeIDs.Num() <= 1)
	{
		return;
	}

	int32 CurrentTypeIndex = BuildingTypeIDs.IndexOfByKey(CurrentBuildingTypeID);
	if (CurrentTypeIndex == INDEX_NONE)
	{
		CurrentTypeIndex = 0;
	}
	const int32 Step = Direction > 0 ? 1 : -1;
	const int32 NextTypeIndex = (CurrentTypeIndex + Step + BuildingTypeIDs.Num()) % BuildingTypeIDs.Num();
	SelectClosestTierForType(BuildingTypeIDs[NextTypeIndex], CurrentTier);
}

void UCampBuildComponent::ChangeSelectedTier(int32 Direction)
{
	if (Direction == 0)
	{
		return;
	}

	if (BuildingRowNames.IsEmpty())
	{
		LoadBuildingDataRows();
	}

	if (BuildingDataTable == nullptr || CurrentBuildingRowName.IsNone() || CurrentBuildingTypeID.IsNone())
	{
		return;
	}

	if (GetSelectedBuildingData() == nullptr)
	{
		return;
	}

	const int32 NextTier = CurrentTier + (Direction > 0 ? 1 : -1);
	SelectTierForCurrentType(NextTier);
}

void UCampBuildComponent::UpdateBuildTrace()
{
	FHitResult HitResult;
	bHasCurrentTraceHit = TraceFromCamera(HitResult);

	if (bHasCurrentTraceHit)
	{
		CurrentTraceHit = HitResult;
		CurrentGridIndex = WorldLocationToGridIndex(CurrentTraceHit.ImpactPoint);
		CurrentSnappedLocation = GridIndexToWorldLocation(CurrentGridIndex);
		UpdateSurfaceTraces();
		UpdatePlacementBoxTrace();
	}
	else
	{
		bCanPlaceCurrentPreview = false;
		bSurfacePlacementBlocked = false;
	}

	UpdateBuildPreviewActor();
}

bool UCampBuildComponent::TraceFromCamera(FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (World == nullptr || OwnerPawn == nullptr)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (PC == nullptr || PC->PlayerCameraManager == nullptr)
	{
		return false;
	}

	const FVector TraceStart = PC->PlayerCameraManager->GetCameraLocation();
	const FVector TraceEnd = TraceStart + PC->PlayerCameraManager->GetCameraRotation().Vector() * CameraTraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	return World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, BuildTraceChannel, QueryParams);
}

void UCampBuildComponent::LoadBuildingDataRows()
{
	BuildingRowNames.Empty();
	BuildingTypeIDs.Empty();

	if (BuildingDataTable == nullptr)
	{
		SelectedBuildingIndex = INDEX_NONE;
		CurrentBuildingRowName = NAME_None;
		CurrentBuildingTypeID = NAME_None;
		CurrentTier = 1;
		CurrentBuildSize = FVector::ZeroVector;
		return;
	}

	BuildingRowNames = BuildingDataTable->GetRowNames();

	for (int32 Index = 0; Index < BuildingRowNames.Num(); ++Index)
	{
		const FName RowName = BuildingRowNames[Index];
		const FBuildingDataRow* BuildingData = BuildingDataTable->FindRow<FBuildingDataRow>(RowName, TEXT("CampBuildComponent::LoadBuildingDataRows"));
		if (BuildingData == nullptr)
		{
			continue;
		}

		if (!BuildingData->BuildingTypeID.IsNone() && !BuildingTypeIDs.Contains(BuildingData->BuildingTypeID))
		{
			BuildingTypeIDs.Add(BuildingData->BuildingTypeID);
		}
	}

	if (BuildingRowNames.IsEmpty())
	{
		SelectedBuildingIndex = INDEX_NONE;
		CurrentBuildingRowName = NAME_None;
		CurrentBuildingTypeID = NAME_None;
		CurrentTier = 1;
		CurrentBuildSize = FVector::ZeroVector;
	}
}

bool UCampBuildComponent::SelectBuildingByIndex(int32 BuildingIndex)
{
	if (BuildingDataTable == nullptr || !BuildingRowNames.IsValidIndex(BuildingIndex))
	{
		return false;
	}

	const FName RowName = BuildingRowNames[BuildingIndex];
	const FBuildingDataRow* BuildingData = BuildingDataTable->FindRow<FBuildingDataRow>(RowName, TEXT("CampBuildComponent::SelectBuildingByIndex"));
	if (BuildingData == nullptr)
	{
		return false;
	}

	TSubclassOf<ABuildableActor> LoadedBuildActorClass = BuildingData->BuildActorClass.LoadSynchronous();
	if (LoadedBuildActorClass == nullptr)
	{
		return false;
	}

	SelectedBuildingIndex = BuildingIndex;
	CurrentBuildingRowName = RowName;
	CurrentBuildingTypeID = BuildingData->BuildingTypeID;
	CurrentTier = FMath::Max(BuildingData->Tier, 1);
	BuildActorClass = LoadedBuildActorClass;
	CurrentBuildSize = FVector(
		FMath::Max(BuildingData->SizeX, 1.0f),
		FMath::Max(BuildingData->SizeY, 1.0f),
		FMath::Max(BuildingData->SizeZ, 1.0f));

	if (CurrentPreviewActor != nullptr)
	{
		DestroyBuildPreviewActor();
		SpawnBuildPreviewActor();
	}

	return true;
}

bool UCampBuildComponent::SelectBuildingByRowName(FName RowName)
{
	const int32 BuildingIndex = FindBuildingIndexByRowName(RowName);
	if (BuildingIndex == INDEX_NONE)
	{
		return false;
	}

	return SelectBuildingByIndex(BuildingIndex);
}

int32 UCampBuildComponent::FindBuildingIndexByRowName(FName RowName) const
{
	return BuildingRowNames.IndexOfByKey(RowName);
}

bool UCampBuildComponent::SelectClosestTierForType(FName BuildingTypeID, int32 DesiredTier)
{
	int32 BestIndex = INDEX_NONE;
	int32 BestTierDistance = MAX_int32;
	int32 BestTier = MAX_int32;

	for (int32 Index = 0; Index < BuildingRowNames.Num(); ++Index)
	{
		const FBuildingDataRow* BuildingData = BuildingDataTable != nullptr
			? BuildingDataTable->FindRow<FBuildingDataRow>(BuildingRowNames[Index], TEXT("CampBuildComponent::SelectClosestTierForType"))
			: nullptr;
		if (BuildingData == nullptr || BuildingData->BuildingTypeID != BuildingTypeID)
		{
			continue;
		}

		const int32 TierDistance = FMath::Abs(BuildingData->Tier - DesiredTier);
		const bool bBetterDistance = TierDistance < BestTierDistance;
		const bool bSameDistanceLowerTier = TierDistance == BestTierDistance && BuildingData->Tier < BestTier;
		if (bBetterDistance || bSameDistanceLowerTier)
		{
			BestIndex = Index;
			BestTierDistance = TierDistance;
			BestTier = BuildingData->Tier;
		}
	}

	return BestIndex != INDEX_NONE && SelectBuildingByIndex(BestIndex);
}

bool UCampBuildComponent::SelectTierForCurrentType(int32 Tier)
{
	if (Tier < 1)
	{
		return false;
	}

	for (int32 Index = 0; Index < BuildingRowNames.Num(); ++Index)
	{
		const FBuildingDataRow* BuildingData = BuildingDataTable != nullptr
			? BuildingDataTable->FindRow<FBuildingDataRow>(BuildingRowNames[Index], TEXT("CampBuildComponent::SelectTierForCurrentType"))
			: nullptr;
		if (BuildingData != nullptr && BuildingData->BuildingTypeID == CurrentBuildingTypeID && BuildingData->Tier == Tier)
		{
			return SelectBuildingByIndex(Index);
		}
	}

	return false;
}

const FBuildingDataRow* UCampBuildComponent::GetSelectedBuildingData() const
{
	if (BuildingDataTable == nullptr || CurrentBuildingRowName.IsNone())
	{
		return nullptr;
	}

	return BuildingDataTable->FindRow<FBuildingDataRow>(CurrentBuildingRowName, TEXT("CampBuildComponent::GetSelectedBuildingData"));
}

FVector UCampBuildComponent::GetSelectedBuildSize() const
{
	if (!CurrentBuildSize.IsNearlyZero())
	{
		return CurrentBuildSize;
	}

	if (const FBuildingDataRow* BuildingData = GetSelectedBuildingData())
	{
		return FVector(
			FMath::Max(BuildingData->SizeX, 1.0f),
			FMath::Max(BuildingData->SizeY, 1.0f),
			FMath::Max(BuildingData->SizeZ, 1.0f));
	}

	return FVector::ZeroVector;
}

FVector2D UCampBuildComponent::GetBuildFootprintSize() const
{
	const FVector SelectedBuildSize = GetSelectedBuildSize();
	if (!SelectedBuildSize.IsNearlyZero())
	{
		return FVector2D(SelectedBuildSize.X, SelectedBuildSize.Y);
	}

	const ABuildableActor* BuildDefault = BuildActorClass != nullptr ? BuildActorClass->GetDefaultObject<ABuildableActor>() : nullptr;
	if (BuildDefault != nullptr)
	{
		return FVector2D(
			FMath::Max(BuildDefault->FootprintSize.X, 1.0f),
			FMath::Max(BuildDefault->FootprintSize.Y, 1.0f));
	}

	return FVector2D(FMath::Max(GridCellSize, 1.0f), FMath::Max(GridCellSize, 1.0f));
}

FVector2D UCampBuildComponent::GetGridAlignedFootprintSize() const
{
	const FVector2D FootprintSize = GetBuildFootprintSize();
	const float SafeCellSize = FMath::Max(GridCellSize, 1.0f);

	return FVector2D(
		FMath::CeilToFloat(FootprintSize.X / SafeCellSize) * SafeCellSize,
		FMath::CeilToFloat(FootprintSize.Y / SafeCellSize) * SafeCellSize);
}

FVector2D UCampBuildComponent::GetPlacementHalfExtent() const
{
	return GetGridAlignedFootprintSize() * 0.5f;
}

float UCampBuildComponent::GetPlacementBoxHalfHeight() const
{
	const FVector SelectedBuildSize = GetSelectedBuildSize();
	if (!SelectedBuildSize.IsNearlyZero())
	{
		return FMath::Max(SelectedBuildSize.Z * 0.5f, 1.0f);
	}

	const ABuildableActor* BuildDefault = BuildActorClass != nullptr ? BuildActorClass->GetDefaultObject<ABuildableActor>() : nullptr;
	return BuildDefault != nullptr ? FMath::Max(BuildDefault->PlacementBoxHalfHeight, 1.0f) : 100.0f;
}

float UCampBuildComponent::GetBuildActorHalfHeight() const
{
	const FVector SelectedBuildSize = GetSelectedBuildSize();
	if (!SelectedBuildSize.IsNearlyZero())
	{
		return FMath::Max(SelectedBuildSize.Z * 0.5f, 0.0f);
	}

	const ABuildableActor* BuildDefault = BuildActorClass != nullptr ? BuildActorClass->GetDefaultObject<ABuildableActor>() : nullptr;
	return BuildDefault != nullptr ? FMath::Max(BuildDefault->ActorHalfHeight, 0.0f) : 50.0f;
}

FIntPoint UCampBuildComponent::WorldLocationToGridIndex(const FVector& WorldLocation) const
{
	const float SafeCellSize = FMath::Max(GridCellSize, 1.0f);
	const FVector LocalLocation = WorldLocation - GridOrigin;

	return FIntPoint(
		FMath::RoundToInt(LocalLocation.X / SafeCellSize),
		FMath::RoundToInt(LocalLocation.Y / SafeCellSize)
	);
}

FVector UCampBuildComponent::GridIndexToWorldLocation(const FIntPoint& GridIndex) const
{
	const float SafeCellSize = FMath::Max(GridCellSize, 1.0f);

	return GridOrigin + FVector(
		static_cast<float>(GridIndex.X) * SafeCellSize,
		static_cast<float>(GridIndex.Y) * SafeCellSize,
		CurrentTraceHit.ImpactPoint.Z - GridOrigin.Z);
}

void UCampBuildComponent::UpdateSurfaceTraces()
{
	bSurfacePlacementBlocked = false;
	const FVector2D PlacementHalfExtent = GetPlacementHalfExtent();
	const FQuat PlacementRotation = FRotator(0.0f, CurrentBuildYaw, 0.0f).Quaternion();

	FHitResult CenterHit;
	const bool bCenterHit = TraceSurfaceAtLocation(CurrentSnappedLocation, CenterHit);

	bSurfacePlacementBlocked = bSurfacePlacementBlocked || !bCenterHit || IsBlockedSurfaceHit(CenterHit);

	if (bCenterHit)
	{
		CurrentSnappedLocation.Z = CenterHit.ImpactPoint.Z;
	}

	const FVector CornerOffsets[] =
	{
		FVector(PlacementHalfExtent.X, PlacementHalfExtent.Y, 0.0f),
		FVector(PlacementHalfExtent.X, -PlacementHalfExtent.Y, 0.0f),
		FVector(-PlacementHalfExtent.X, PlacementHalfExtent.Y, 0.0f),
		FVector(-PlacementHalfExtent.X, -PlacementHalfExtent.Y, 0.0f),
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(CornerOffsets); ++Index)
	{
		FHitResult SurfaceHit;
		const FVector TraceLocation = CurrentSnappedLocation + PlacementRotation.RotateVector(CornerOffsets[Index]);
		const bool bSurfaceHit = TraceSurfaceAtLocation(TraceLocation, SurfaceHit);

		bSurfacePlacementBlocked = bSurfacePlacementBlocked || IsUnsupportedSurfaceHit(bSurfaceHit, SurfaceHit);
	}
}

bool UCampBuildComponent::TraceSurfaceAtLocation(const FVector& TraceLocation, FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (World == nullptr)
	{
		return false;
	}

	const float SafeTraceHeight = FMath::Max(SurfaceTraceHeight, 1.0f);
	const FVector TraceStart = TraceLocation + FVector(0.0f, 0.0f, SafeTraceHeight);
	const FVector TraceEnd = TraceLocation - FVector(0.0f, 0.0f, SafeTraceHeight);

	FCollisionQueryParams QueryParams;
	if (OwnerPawn != nullptr)
	{
		QueryParams.AddIgnoredActor(OwnerPawn);
	}
	if (CurrentPreviewActor != nullptr)
	{
		QueryParams.AddIgnoredActor(CurrentPreviewActor);
	}

	return World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, BuildTraceChannel, QueryParams);
}

bool UCampBuildComponent::IsBlockedSurfaceHit(const FHitResult& HitResult) const
{
	const AActor* HitActor = HitResult.GetActor();
	return HitActor != nullptr && HitActor->IsA<ABuildableActor>();
}

bool UCampBuildComponent::IsUnsupportedSurfaceHit(bool bHit, const FHitResult& HitResult) const
{
	if (!bHit)
	{
		return true;
	}

	if (IsBlockedSurfaceHit(HitResult))
	{
		return true;
	}

	return FMath::Abs(HitResult.ImpactPoint.Z - CurrentSnappedLocation.Z) > FMath::Max(SurfaceHeightTolerance, 0.0f);
}

void UCampBuildComponent::UpdatePlacementBoxTrace()
{
	UWorld* World = GetWorld();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (World == nullptr)
	{
		bCanPlaceCurrentPreview = false;
		return;
	}

	const FVector2D PlacementHalfExtent = GetPlacementHalfExtent();
	const float SafeExtentShrink = FMath::Max(PlacementBoxExtentShrink, 0.0f);
	const FVector BoxExtent(
		FMath::Max(PlacementHalfExtent.X - SafeExtentShrink, 1.0f),
		FMath::Max(PlacementHalfExtent.Y - SafeExtentShrink, 1.0f),
		GetPlacementBoxHalfHeight());

	const FVector BoxCenter = CurrentSnappedLocation + FVector(0.0f, 0.0f, BoxExtent.Z + PlacementBoxGroundClearance);
	const FQuat BoxRotation = FRotator(0.0f, CurrentBuildYaw, 0.0f).Quaternion();

	FCollisionQueryParams QueryParams;
	if (OwnerPawn != nullptr)
	{
		QueryParams.AddIgnoredActor(OwnerPawn);
	}
	if (CurrentPreviewActor != nullptr)
	{
		QueryParams.AddIgnoredActor(CurrentPreviewActor);
	}

	TArray<FOverlapResult> BoxOverlaps;
	World->OverlapMultiByChannel(
		BoxOverlaps,
		BoxCenter,
		BoxRotation,
		BuildTraceChannel,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams);

	bool bHasBlockingOverlap = false;
	for (const FOverlapResult& BoxOverlap : BoxOverlaps)
	{
		const AActor* HitActor = BoxOverlap.GetActor();
		if (HitActor == nullptr)
		{
			continue;
		}

		bHasBlockingOverlap = true;
		break;
	}

	if (!bHasBlockingOverlap)
	{
		bCanPlaceCurrentPreview = !bSurfacePlacementBlocked;
		return;
	}

	bCanPlaceCurrentPreview = false;
}

void UCampBuildComponent::SpawnBuildPreviewActor()
{
	if (CurrentPreviewActor != nullptr || BuildActorClass == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentPreviewActor = World->SpawnActor<ABuildableActor>(BuildActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentPreviewActor != nullptr)
	{
		const FVector SelectedBuildSize = GetSelectedBuildSize();
		if (!SelectedBuildSize.IsNearlyZero())
		{
			CurrentPreviewActor->SetBuildSize(SelectedBuildSize);
		}
		if (const FBuildingDataRow* BuildingData = GetSelectedBuildingData())
		{
			if (UStaticMesh* StaticMesh = BuildingData->StaticMesh.LoadSynchronous())
			{
				CurrentPreviewActor->SetBuildMesh(StaticMesh);
			}
		}
		CurrentPreviewActor->SetPreviewMode(true);
		CurrentPreviewActor->SetActorHiddenInGame(true);
	}
}

void UCampBuildComponent::DestroyBuildPreviewActor()
{
	if (CurrentPreviewActor != nullptr)
	{
		CurrentPreviewActor->Destroy();
		CurrentPreviewActor = nullptr;
	}
}

void UCampBuildComponent::UpdateBuildPreviewActor()
{
	if (CurrentPreviewActor == nullptr)
	{
		return;
	}

	if (!bHasCurrentTraceHit)
	{
		CurrentPreviewActor->SetActorHiddenInGame(true);
		return;
	}

	const FVector PreviewLocation = CurrentSnappedLocation + FVector(0.0f, 0.0f, GetBuildActorHalfHeight());
	CurrentPreviewActor->SetActorLocation(PreviewLocation);
	CurrentPreviewActor->SetActorRotation(FRotator(0.0f, CurrentBuildYaw, 0.0f));
	CurrentPreviewActor->SetActorHiddenInGame(false);
	CurrentPreviewActor->SetPlacementValid(bCanPlaceCurrentPreview);
}
