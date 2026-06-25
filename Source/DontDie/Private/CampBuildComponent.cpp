// Fill out your copyright notice in the Description page of Project Settings.


#include "CampBuildComponent.h"

#include "BuildingSelectionComponent.h"
#include "BuildingDataRow.h"
#include "BuildableActor.h"
#include "Camera/PlayerCameraManager.h"
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
		if (bBuildEditMode)
		{
			UpdateBuildEditTrace();
		}
		else
		{
			UpdateBuildTrace();
		}
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
		bBuildEditMode = false;
		ClearHoveredBuildableActor();
		ClearSelectedBuildableActor();
		DestroyBuildPreviewActor();
	}
	else
	{
		SyncLegacyBuildingDataTable();
		SelectInitialBuilding();

		SpawnBuildPreviewActor();
	}
}

void UCampBuildComponent::SetBuildingSelectionComponent(UBuildingSelectionComponent* NewBuildingSelectionComponent)
{
	BuildingSelectionComponent = NewBuildingSelectionComponent;
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

void UCampBuildComponent::ToggleBuildEditMode()
{
	SetBuildEditMode(!bBuildEditMode);
}

void UCampBuildComponent::SetBuildEditMode(bool bEnabled)
{
	if (!bBuildMode || bBuildEditMode == bEnabled)
	{
		return;
	}

	bBuildEditMode = bEnabled;
	if (bBuildEditMode)
	{
		bHasCurrentTraceHit = false;
		bCanPlaceCurrentPreview = false;
		DestroyBuildPreviewActor();
	}
	else
	{
		ClearHoveredBuildableActor();
		ClearSelectedBuildableActor();
		SpawnBuildPreviewActor();
		UpdateBuildPreviewActor();
	}
}

bool UCampBuildComponent::IsBuildEditModeEnabled() const
{
	return bBuildEditMode;
}

bool UCampBuildComponent::DeleteSelectedBuild()
{
	if (!bBuildMode || !bBuildEditMode)
	{
		return false;
	}

	ABuildableActor* DeleteTarget = SelectedBuildableActor != nullptr ? SelectedBuildableActor : HoveredBuildableActor;
	if (DeleteTarget == nullptr)
	{
		return false;
	}

	if (DeleteTarget == HoveredBuildableActor)
	{
		HoveredBuildableActor = nullptr;
	}
	if (DeleteTarget == SelectedBuildableActor)
	{
		SelectedBuildableActor = nullptr;
	}

	DeleteTarget->SetDemolitionPreview(false);
	DeleteTarget->Destroy();
	return true;
}

bool UCampBuildComponent::MoveSelectedBuild()
{
	if (!bBuildMode || !bBuildEditMode)
	{
		return false;
	}

	SelectHoveredBuildable();
	return SelectedBuildableActor != nullptr;
}

bool UCampBuildComponent::EditSelectedBuild()
{
	if (!bBuildMode || !bBuildEditMode)
	{
		return false;
	}

	SelectHoveredBuildable();
	return SelectedBuildableActor != nullptr;
}

bool UCampBuildComponent::ConfirmBuild()
{
	if (!bBuildMode)
	{
		return false;
	}

	if (bBuildEditMode)
	{
		SelectHoveredBuildable();
		return SelectedBuildableActor != nullptr;
	}

	if (!bHasCurrentTraceHit)
	{
		return false;
	}

	if (!bCanPlaceCurrentPreview)
	{
		return false;
	}

	if (GetSelectedBuildActorClass() == nullptr)
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
	if (bBuildEditMode)
	{
		return;
	}

	CurrentBuildYaw = FMath::Fmod(CurrentBuildYaw + YawDelta, 360.0f);
	if (CurrentBuildYaw < 0.0f)
	{
		CurrentBuildYaw += 360.0f;
	}

	UpdateBuildPreviewActor();
}

void UCampBuildComponent::ChangeSelectedBuilding(int32 Direction)
{
	if (bBuildEditMode)
	{
		return;
	}

	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent == nullptr)
	{
		return;
	}

	SelectionComponent->ChangeSelectedBuilding(Direction);
	DestroyBuildPreviewActor();
	SpawnBuildPreviewActor();
	UpdateBuildPreviewActor();
}

void UCampBuildComponent::ChangeSelectedTier(int32 Direction)
{
	if (bBuildEditMode)
	{
		return;
	}

	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent == nullptr)
	{
		return;
	}

	SelectionComponent->ChangeSelectedTier(Direction);
	DestroyBuildPreviewActor();
	SpawnBuildPreviewActor();
	UpdateBuildPreviewActor();
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

void UCampBuildComponent::UpdateBuildEditTrace()
{
	FHitResult HitResult;
	const bool bHit = TraceFromCamera(HitResult);
	ABuildableActor* HitBuildableActor = bHit ? Cast<ABuildableActor>(HitResult.GetActor()) : nullptr;

	SetHoveredBuildableActor(HitBuildableActor);
	bHasCurrentTraceHit = HitBuildableActor != nullptr;
	if (bHasCurrentTraceHit)
	{
		CurrentTraceHit = HitResult;
	}
}

void UCampBuildComponent::SelectHoveredBuildable()
{
	SelectedBuildableActor = HoveredBuildableActor;
}

void UCampBuildComponent::SetHoveredBuildableActor(ABuildableActor* NewHoveredBuildableActor)
{
	if (HoveredBuildableActor == NewHoveredBuildableActor)
	{
		return;
	}

	ClearHoveredBuildableActor();
	HoveredBuildableActor = NewHoveredBuildableActor;
	if (HoveredBuildableActor != nullptr)
	{
		HoveredBuildableActor->SetDemolitionPreview(true);
	}
}

void UCampBuildComponent::ClearHoveredBuildableActor()
{
	if (HoveredBuildableActor != nullptr)
	{
		HoveredBuildableActor->SetDemolitionPreview(false);
		HoveredBuildableActor = nullptr;
	}
}

void UCampBuildComponent::ClearSelectedBuildableActor()
{
	SelectedBuildableActor = nullptr;
}

UBuildingSelectionComponent* UCampBuildComponent::GetBuildingSelectionComponent()
{
	if (BuildingSelectionComponent == nullptr)
	{
		BuildingSelectionComponent = GetOwner() != nullptr ? GetOwner()->FindComponentByClass<UBuildingSelectionComponent>() : nullptr;
	}

	return BuildingSelectionComponent;
}

const UBuildingSelectionComponent* UCampBuildComponent::GetBuildingSelectionComponent() const
{
	return BuildingSelectionComponent != nullptr
		? BuildingSelectionComponent
		: (GetOwner() != nullptr ? GetOwner()->FindComponentByClass<UBuildingSelectionComponent>() : nullptr);
}

void UCampBuildComponent::SyncLegacyBuildingDataTable()
{
	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent != nullptr && SelectionComponent->BuildingDataTable == nullptr)
	{
		SelectionComponent->BuildingDataTable = BuildingDataTable;
	}
}

bool UCampBuildComponent::SelectInitialBuilding()
{
	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent == nullptr)
	{
		return false;
	}

	SelectionComponent->LoadBuildingDataRows();
	if (SelectionComponent->GetBuildingRowNames().IsValidIndex(SelectionComponent->GetSelectedBuildingIndex()))
	{
		return SelectionComponent->SelectBuildingByIndex(SelectionComponent->GetSelectedBuildingIndex());
	}

	return !SelectionComponent->GetBuildingRowNames().IsEmpty() && SelectionComponent->SelectBuildingByIndex(0);
}

const FBuildingDataRow* UCampBuildComponent::GetSelectedBuildingData() const
{
	const UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	return SelectionComponent != nullptr ? SelectionComponent->GetSelectedBuildingData() : nullptr;
}

FVector UCampBuildComponent::GetSelectedBuildSize() const
{
	const UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	return SelectionComponent != nullptr ? SelectionComponent->GetSelectedBuildSize() : FVector::ZeroVector;
}

TSubclassOf<ABuildableActor> UCampBuildComponent::GetSelectedBuildActorClass() const
{
	const UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	return SelectionComponent != nullptr ? SelectionComponent->GetSelectedBuildActorClass() : nullptr;
}

FVector2D UCampBuildComponent::GetBuildFootprintSize() const
{
	const FVector SelectedBuildSize = GetSelectedBuildSize();
	if (!SelectedBuildSize.IsNearlyZero())
	{
		return FVector2D(SelectedBuildSize.X, SelectedBuildSize.Y);
	}

	const TSubclassOf<ABuildableActor> SelectedBuildActorClass = GetSelectedBuildActorClass();
	const ABuildableActor* BuildDefault = SelectedBuildActorClass != nullptr ? SelectedBuildActorClass->GetDefaultObject<ABuildableActor>() : nullptr;
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

	const TSubclassOf<ABuildableActor> SelectedBuildActorClass = GetSelectedBuildActorClass();
	const ABuildableActor* BuildDefault = SelectedBuildActorClass != nullptr ? SelectedBuildActorClass->GetDefaultObject<ABuildableActor>() : nullptr;
	return BuildDefault != nullptr ? FMath::Max(BuildDefault->PlacementBoxHalfHeight, 1.0f) : 100.0f;
}

float UCampBuildComponent::GetBuildActorHalfHeight() const
{
	const FVector SelectedBuildSize = GetSelectedBuildSize();
	if (!SelectedBuildSize.IsNearlyZero())
	{
		return FMath::Max(SelectedBuildSize.Z * 0.5f, 0.0f);
	}

	const TSubclassOf<ABuildableActor> SelectedBuildActorClass = GetSelectedBuildActorClass();
	const ABuildableActor* BuildDefault = SelectedBuildActorClass != nullptr ? SelectedBuildActorClass->GetDefaultObject<ABuildableActor>() : nullptr;
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
	const TSubclassOf<ABuildableActor> SelectedBuildActorClass = GetSelectedBuildActorClass();
	if (CurrentPreviewActor != nullptr || SelectedBuildActorClass == nullptr)
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

	CurrentPreviewActor = World->SpawnActor<ABuildableActor>(SelectedBuildActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
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
