// Fill out your copyright notice in the Description page of Project Settings.


#include "CampBuildComponent.h"

#include "BuildingDataRow.h"
#include "BuildableActor.h"
#include "DrawDebugHelpers.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
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
		CurrentSurfaceDebugText.Empty();
		CurrentBoxTraceDebugText.Empty();
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
		const FString Message = TEXT("CampBuildComponent: Build mode is disabled.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Message);
		}
		return false;
	}

	if (!bHasCurrentTraceHit)
	{
		const FString Message = TEXT("CampBuildComponent: No build trace hit.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Message);
		}
		return false;
	}

	if (!bCanPlaceCurrentPreview)
	{
		const FString Message = FString::Printf(TEXT("CampBuildComponent: Current placement is blocked.%s"), *CurrentBoxTraceDebugText);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Message);
		}
		return false;
	}

	if (BuildActorClass == nullptr)
	{
		const FString Message = TEXT("CampBuildComponent: BuildActorClass is not assigned.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Message);
		}
		return false;
	}

	if (CurrentPreviewActor == nullptr)
	{
		const FString Message = TEXT("CampBuildComponent: Preview actor is not spawned.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Message);
		}
		return false;
	}

	ABuildableActor* BuiltActor = CurrentPreviewActor;
	CurrentPreviewActor = nullptr;
	BuiltActor->FinalizeBuild();

	const FVector BuildLocation = BuiltActor->GetActorLocation();
	const FString Message = FString::Printf(
		TEXT("CampBuildComponent: Build confirmed. Actor=%s Location=(%.0f, %.0f, %.0f)"),
		*BuiltActor->GetName(),
		BuildLocation.X,
		BuildLocation.Y,
		BuildLocation.Z);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, Message);
	}

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
		const FString Message = TEXT("CampBuildComponent: ChangeSelectedBuilding failed. No building types loaded.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
		}
		return;
	}

	if (BuildingTypeIDs.Num() <= 1)
	{
		const FString Message = FString::Printf(TEXT("CampBuildComponent: ChangeSelectedBuilding ignored. Type count=%d."), BuildingTypeIDs.Num());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
		}
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
		const FString Message = TEXT("CampBuildComponent: ChangeSelectedTier failed. No selected building data.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
		}
		return;
	}

	if (GetSelectedBuildingData() == nullptr)
	{
		const FString Message = FString::Printf(TEXT("CampBuildComponent: ChangeSelectedTier failed. Row=%s is invalid."), *CurrentBuildingRowName.ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
		}
		return;
	}

	const int32 NextTier = CurrentTier + (Direction > 0 ? 1 : -1);
	if (!SelectTierForCurrentType(NextTier))
	{
		const FString Message = FString::Printf(TEXT("CampBuildComponent: No tier %d for type %s."), NextTier, *CurrentBuildingTypeID.ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
		}
	}
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
		CurrentSurfaceDebugText.Empty();
		CurrentBoxTraceDebugText.Empty();
	}

	UpdateBuildPreviewActor();
	DrawCurrentTraceDebug();
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

	const bool bHit = World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, BuildTraceChannel, QueryParams);

	if (bDrawDebugTrace)
	{
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, false, 0.0f, 0, 2.0f);

		if (bHit)
		{
			DrawDebugSphere(World, OutHit.ImpactPoint, 12.0f, 8, TraceColor, false, 0.0f);
		}
	}

	return bHit;
}

void UCampBuildComponent::LoadBuildingDataRows()
{
	BuildingRowNames.Empty();
	BuildingTypeIDs.Empty();

	if (BuildingDataTable == nullptr)
	{
		const FString Message = TEXT("CampBuildComponent: BuildingDataTable is not assigned. Using BuildActorClass fallback.");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Message);
		}

		SelectedBuildingIndex = INDEX_NONE;
		CurrentBuildingRowName = NAME_None;
		CurrentBuildingTypeID = NAME_None;
		CurrentTier = 1;
		CurrentBuildSize = FVector::ZeroVector;
		return;
	}

	BuildingRowNames = BuildingDataTable->GetRowNames();
	const FString LoadedMessage = FString::Printf(
		TEXT("CampBuildComponent: Loaded %d building row(s) from %s."),
		BuildingRowNames.Num(),
		*BuildingDataTable->GetName());
	UE_LOG(LogTemp, Log, TEXT("%s"), *LoadedMessage);
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, LoadedMessage);
	}

	for (int32 Index = 0; Index < BuildingRowNames.Num(); ++Index)
	{
		const FName RowName = BuildingRowNames[Index];
		const FBuildingDataRow* BuildingData = BuildingDataTable->FindRow<FBuildingDataRow>(RowName, TEXT("CampBuildComponent::LoadBuildingDataRows"));
		if (BuildingData == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("CampBuildComponent: [%d] Row=%s is invalid."), Index, *RowName.ToString());
			continue;
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("CampBuildComponent: [%d] Row=%s Type=%s Tier=%d DisplayName=%s Class=%s Mesh=%s Size=(%.0f, %.0f, %.0f)"),
			Index,
			*RowName.ToString(),
			*BuildingData->BuildingTypeID.ToString(),
			BuildingData->Tier,
			*BuildingData->DisplayName.ToString(),
			*BuildingData->BuildActorClass.ToString(),
			*BuildingData->StaticMesh.ToString(),
			BuildingData->SizeX,
			BuildingData->SizeY,
			BuildingData->SizeZ);

		if (!BuildingData->BuildingTypeID.IsNone() && !BuildingTypeIDs.Contains(BuildingData->BuildingTypeID))
		{
			BuildingTypeIDs.Add(BuildingData->BuildingTypeID);
		}
	}

	if (BuildingRowNames.IsEmpty())
	{
		const FString Message = FString::Printf(TEXT("CampBuildComponent: BuildingDataTable %s has no rows."), *BuildingDataTable->GetName());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Message);
		}

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
		UE_LOG(LogTemp, Warning, TEXT("CampBuildComponent: SelectBuildingByIndex failed. Index=%d RowCount=%d"), BuildingIndex, BuildingRowNames.Num());
		return false;
	}

	const FName RowName = BuildingRowNames[BuildingIndex];
	const FBuildingDataRow* BuildingData = BuildingDataTable->FindRow<FBuildingDataRow>(RowName, TEXT("CampBuildComponent::SelectBuildingByIndex"));
	if (BuildingData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("CampBuildComponent: SelectBuildingByIndex failed. Row=%s is invalid."), *RowName.ToString());
		return false;
	}

	TSubclassOf<ABuildableActor> LoadedBuildActorClass = BuildingData->BuildActorClass.LoadSynchronous();
	if (LoadedBuildActorClass == nullptr)
	{
		const FString Message = FString::Printf(TEXT("CampBuildComponent: BuildActorClass is not assigned. Row=%s"), *RowName.ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Message);
		}
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

	const FString Message = FString::Printf(
		TEXT("CampBuildComponent: Selected building [%d] Row=%s Type=%s Tier=%d DisplayName=%s Class=%s Size=(%.0f, %.0f, %.0f)"),
		SelectedBuildingIndex,
		*CurrentBuildingRowName.ToString(),
		*CurrentBuildingTypeID.ToString(),
		CurrentTier,
		*BuildingData->DisplayName.ToString(),
		*BuildActorClass->GetName(),
		CurrentBuildSize.X,
		CurrentBuildSize.Y,
		CurrentBuildSize.Z);
	UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, Message);
	}

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
		const FString Message = FString::Printf(TEXT("CampBuildComponent: SelectBuildingByRowName failed. Row=%s was not found."), *RowName.ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
		}
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
	CurrentSurfaceDebugText.Empty();
	bSurfacePlacementBlocked = false;
	const FVector2D PlacementHalfExtent = GetPlacementHalfExtent();
	const FQuat PlacementRotation = FRotator(0.0f, CurrentBuildYaw, 0.0f).Quaternion();

	FHitResult CenterHit;
	const bool bCenterHit = TraceSurfaceAtLocation(CurrentSnappedLocation, CenterHit);

	CurrentSurfaceDebugText += BuildSurfaceTraceDebugLine(TEXT("Center"), bCenterHit, CenterHit);
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

	const TCHAR* CornerLabels[] =
	{
		TEXT("Corner +X +Y"),
		TEXT("Corner +X -Y"),
		TEXT("Corner -X +Y"),
		TEXT("Corner -X -Y"),
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(CornerOffsets); ++Index)
	{
		FHitResult SurfaceHit;
		const FVector TraceLocation = CurrentSnappedLocation + PlacementRotation.RotateVector(CornerOffsets[Index]);
		const bool bSurfaceHit = TraceSurfaceAtLocation(TraceLocation, SurfaceHit);

		CurrentSurfaceDebugText += TEXT("\n");
		CurrentSurfaceDebugText += BuildSurfaceTraceDebugLine(CornerLabels[Index], bSurfaceHit, SurfaceHit);
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

	const bool bHit = World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, BuildTraceChannel, QueryParams);

	if (bDrawDebugTrace)
	{
		const FColor TraceColor = bHit ? FColor::Blue : FColor::Yellow;
		DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, false, 0.0f, 0, 2.0f);

		if (bHit)
		{
			DrawDebugSphere(World, OutHit.ImpactPoint, 10.0f, 8, TraceColor, false, 0.0f);
		}
	}

	return bHit;
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

FString UCampBuildComponent::BuildSurfaceTraceDebugLine(const TCHAR* Label, bool bHit, const FHitResult& HitResult) const
{
	if (!bHit)
	{
		return FString::Printf(TEXT("%s: No hit NoSurface"), Label);
	}

	const AActor* HitActor = HitResult.GetActor();
	const FString ActorName = HitActor != nullptr ? HitActor->GetName() : TEXT("None");
	const TCHAR* SurfaceState = IsBlockedSurfaceHit(HitResult) ? TEXT(" BlockedSurface") : TEXT("");
	const TCHAR* HeightState = FMath::Abs(HitResult.ImpactPoint.Z - CurrentSnappedLocation.Z) > FMath::Max(SurfaceHeightTolerance, 0.0f) ? TEXT(" UnevenSurface") : TEXT("");

	return FString::Printf(
		TEXT("%s: X=%.0f Y=%.0f Z=%.0f Actor=%s%s%s"),
		Label,
		HitResult.ImpactPoint.X,
		HitResult.ImpactPoint.Y,
		HitResult.ImpactPoint.Z,
		*ActorName,
		SurfaceState,
		HeightState);
}

void UCampBuildComponent::UpdatePlacementBoxTrace()
{
	CurrentBoxTraceDebugText.Empty();

	UWorld* World = GetWorld();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (World == nullptr)
	{
		CurrentBoxTraceDebugText = TEXT("\nBox: No world");
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
	const bool bHit = World->OverlapMultiByChannel(
		BoxOverlaps,
		BoxCenter,
		BoxRotation,
		BuildTraceChannel,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams);

	if (bDrawDebugTrace)
	{
		DrawDebugBox(
			World,
			BoxCenter,
			BoxExtent,
			BoxRotation,
			bHit ? FColor::Red : FColor::Cyan,
			false,
			0.0f,
			0,
			2.0f);
	}

	TArray<FString> HitActorNames;
	for (const FOverlapResult& BoxOverlap : BoxOverlaps)
	{
		const AActor* HitActor = BoxOverlap.GetActor();
		if (HitActor == nullptr)
		{
			continue;
		}

		const FString ActorName = HitActor->GetName();
		if (!HitActorNames.Contains(ActorName))
		{
			HitActorNames.Add(ActorName);
		}
	}

	if (HitActorNames.IsEmpty())
	{
		CurrentBoxTraceDebugText = TEXT("\nBox: No actor");
		bCanPlaceCurrentPreview = !bSurfacePlacementBlocked;
		if (bSurfacePlacementBlocked)
		{
			CurrentBoxTraceDebugText += TEXT("\nSurface: Buildable actor below");
		}
		return;
	}

	bCanPlaceCurrentPreview = false;
	CurrentBoxTraceDebugText = FString::Printf(TEXT("\nBox: %d actor(s)"), HitActorNames.Num());
	for (const FString& ActorName : HitActorNames)
	{
		CurrentBoxTraceDebugText += FString::Printf(TEXT("\n- %s"), *ActorName);
	}
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

void UCampBuildComponent::DrawCurrentTraceDebug() const
{
	if (!bDrawDebugText)
	{
		return;
	}

	const FString DebugText = bHasCurrentTraceHit
		? FString::Printf(
			TEXT("Placement: %s\nHit: X=%.0f Y=%.0f Z=%.0f\nGrid: X=%d Y=%d\nSnap: X=%.0f Y=%.0f\nSize: %.0f x %.0f\nOccupied: %.0f x %.0f\n%s%s"),
			bCanPlaceCurrentPreview ? TEXT("Available") : TEXT("Blocked"),
			CurrentTraceHit.ImpactPoint.X,
			CurrentTraceHit.ImpactPoint.Y,
			CurrentTraceHit.ImpactPoint.Z,
			CurrentGridIndex.X,
			CurrentGridIndex.Y,
			CurrentSnappedLocation.X,
			CurrentSnappedLocation.Y,
			GetBuildFootprintSize().X,
			GetBuildFootprintSize().Y,
			GetGridAlignedFootprintSize().X,
			GetGridAlignedFootprintSize().Y,
			*CurrentSurfaceDebugText,
			*CurrentBoxTraceDebugText)
		: TEXT("No build trace hit");

	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()),
			0.0f,
			bHasCurrentTraceHit ? FColor::Green : FColor::Red,
			DebugText);
	}

	if (bHasCurrentTraceHit)
	{
		DrawDebugString(
			GetWorld(),
			CurrentTraceHit.ImpactPoint + FVector(0.0f, 0.0f, 40.0f),
			DebugText,
			nullptr,
			FColor::White,
			0.0f,
			true);
	}
}
