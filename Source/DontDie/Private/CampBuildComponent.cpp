// Fill out your copyright notice in the Description page of Project Settings.


#include "CampBuildComponent.h"

#include "BuildingSelectionComponent.h"
#include "BuildingDataRow.h"
#include "BuildableActor.h"
#include "Camera/PlayerCameraManager.h"
#include "ConstructionSiteActor.h"
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
		if (BuildState == ECampBuildState::Placement || BuildState == ECampBuildState::Move)
		{
			UpdateBuildTrace();
		}
		else if (BuildState == ECampBuildState::Idle || BuildState == ECampBuildState::Demolition)
		{
			UpdateExistingBuildTrace();
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
		RestoreSelectedBuildableAfterPreview();
		bHasCurrentTraceHit = false;
		bCanPlaceCurrentPreview = false;
		bSurfacePlacementBlocked = false;
		ClearPlacementState();
		ClearExistingBuildState();
		SetBuildState(ECampBuildState::Idle);
	}
	else
	{
		SyncLegacyBuildingDataTable();
		if (UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent())
		{
			SelectionComponent->LoadBuildingDataRows();
		}
		SetBuildState(ECampBuildState::Idle);
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

ECampBuildState UCampBuildComponent::GetBuildState() const
{
	return BuildState;
}

void UCampBuildComponent::ToggleBuildList()
{
	SetBuildListOpen(BuildState != ECampBuildState::BuildList);
}

void UCampBuildComponent::SetBuildListOpen(bool bOpen)
{
	if (!bBuildMode)
	{
		return;
	}

	SetBuildState(bOpen ? ECampBuildState::BuildList : ECampBuildState::Idle);
}

bool UCampBuildComponent::IsBuildListOpen() const
{
	return BuildState == ECampBuildState::BuildList;
}

bool UCampBuildComponent::HasHoveredBuildable() const
{
	return HoveredBuildableActor != nullptr;
}

bool UCampBuildComponent::SelectBuildingByIndex(int32 BuildingIndex)
{
	if (!bBuildMode)
	{
		return false;
	}

	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent == nullptr)
	{
		return false;
	}

	if (SelectionComponent->GetBuildingRowNames().IsEmpty())
	{
		SelectionComponent->LoadBuildingDataRows();
	}

	if (!SelectionComponent->SelectBuildingByIndex(BuildingIndex))
	{
		return false;
	}

	SetBuildState(ECampBuildState::Placement);
	SpawnBuildPreviewActor();
	UpdateBuildPreviewActor();
	return CurrentPreviewActor != nullptr;
}

bool UCampBuildComponent::SelectBuildingByRowName(FName RowName)
{
	if (!bBuildMode)
	{
		return false;
	}

	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent == nullptr)
	{
		return false;
	}

	if (SelectionComponent->GetBuildingRowNames().IsEmpty())
	{
		SelectionComponent->LoadBuildingDataRows();
	}

	if (!SelectionComponent->SelectBuildingByRowName(RowName))
	{
		return false;
	}

	SetBuildState(ECampBuildState::Placement);
	SpawnBuildPreviewActor();
	UpdateBuildPreviewActor();
	return CurrentPreviewActor != nullptr;
}

void UCampBuildComponent::ToggleBuildEditMode()
{
	SetBuildEditMode(BuildState != ECampBuildState::Edit);
}

void UCampBuildComponent::SetBuildEditMode(bool bEnabled)
{
	if (!bBuildMode)
	{
		return;
	}

	if (!bEnabled)
	{
		SetBuildState(ECampBuildState::Idle);
		return;
	}

	if (HoveredBuildableActor == nullptr)
	{
		return;
	}

	SelectHoveredBuildable();
	SetBuildState(ECampBuildState::Edit);
	if (SelectedBuildableActor != nullptr)
	{
		SelectedBuildableActor->SetHoverPreview(true);
	}
}

bool UCampBuildComponent::IsBuildEditModeEnabled() const
{
	return BuildState == ECampBuildState::Edit;
}

bool UCampBuildComponent::DeleteSelectedBuild()
{
	if (!bBuildMode)
	{
		return false;
	}

	if (BuildState == ECampBuildState::Demolition)
	{
		SetBuildState(ECampBuildState::Idle);
		return true;
	}

	if (BuildState != ECampBuildState::Edit)
	{
		SetBuildState(ECampBuildState::Demolition);
		return true;
	}

	ABuildableActor* DeleteTarget = SelectedBuildableActor != nullptr ? SelectedBuildableActor : HoveredBuildableActor;
	if (DeleteTarget == nullptr)
	{
		SetBuildState(ECampBuildState::Demolition);
		return true;
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
	if (!bBuildMode || BuildState != ECampBuildState::Edit)
	{
		return false;
	}

	if (SelectedBuildableActor == nullptr)
	{
		SelectHoveredBuildable();
		if (SelectedBuildableActor == nullptr)
		{
			return false;
		}
	}

	if (!SelectCurrentDataForBuildable(SelectedBuildableActor))
	{
		return false;
	}

	CurrentBuildYaw = SelectedBuildableActor->GetActorRotation().Yaw;
	SetBuildState(ECampBuildState::Move);
	PrepareSelectedBuildableForPreview(true);
	SpawnBuildPreviewActor();
	UpdateModifyPreviewAtSelectedBuildable();
	if (CurrentPreviewActor == nullptr)
	{
		RestoreSelectedBuildableAfterPreview();
		SetBuildState(ECampBuildState::Edit);
	}
	return CurrentPreviewActor != nullptr;
}

bool UCampBuildComponent::EditSelectedBuild()
{
	if (!bBuildMode || BuildState != ECampBuildState::Edit)
	{
		return false;
	}

	if (SelectedBuildableActor == nullptr)
	{
		SelectHoveredBuildable();
		if (SelectedBuildableActor == nullptr)
		{
			return false;
		}
	}

	if (!SelectCurrentDataForBuildable(SelectedBuildableActor))
	{
		return false;
	}

	CurrentBuildYaw = SelectedBuildableActor->GetActorRotation().Yaw;
	SetBuildState(ECampBuildState::Modify);
	PrepareSelectedBuildableForPreview(false);
	SpawnBuildPreviewActor();
	UpdateModifyPreviewAtSelectedBuildable();
	if (CurrentPreviewActor == nullptr)
	{
		RestoreSelectedBuildableAfterPreview();
		SetBuildState(ECampBuildState::Edit);
	}
	return CurrentPreviewActor != nullptr;
}

bool UCampBuildComponent::ConfirmBuild()
{
	if (!bBuildMode)
	{
		return false;
	}

	if (BuildState == ECampBuildState::Edit)
	{
		if (SelectedBuildableActor == nullptr)
		{
			return false;
		}

		return true;
	}

	if (BuildState == ECampBuildState::Demolition)
	{
		ABuildableActor* DeleteTarget = HoveredBuildableActor;
		if (DeleteTarget == nullptr)
		{
			return false;
		}

		HoveredBuildableActor = nullptr;
		if (SelectedBuildableActor == DeleteTarget)
		{
			SelectedBuildableActor = nullptr;
		}
		DeleteTarget->SetDemolitionPreview(false);
		DeleteTarget->Destroy();
		return true;
	}

	if (BuildState == ECampBuildState::Modify)
	{
		return ReplaceSelectedBuildableWithSelectedData();
	}

	if (BuildState == ECampBuildState::Move)
	{
		return MoveSelectedBuildableToPreview();
	}

	if (BuildState != ECampBuildState::Placement)
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

	if (GetSelectedBuildActorClass() == nullptr)
	{
		return false;
	}

	if (CurrentPreviewActor == nullptr)
	{
		return false;
	}

	const FTransform BuildTransform = CurrentPreviewActor->GetActorTransform();
	DestroyBuildPreviewActor();

	AActor* PlacedActor = SpawnPlacedBuildActor(BuildTransform);
	if (PlacedActor == nullptr)
	{
		return false;
	}

	SpawnBuildPreviewActor();
	SetupAutoNextPlacementFromActor(PlacedActor);

	return true;
}

void UCampBuildComponent::CancelBuild()
{
	if (!bBuildMode)
	{
		return;
	}

	if (BuildState == ECampBuildState::Move || BuildState == ECampBuildState::Modify)
	{
		RestoreSelectedBuildableAfterPreview();
		SetBuildState(ECampBuildState::Edit);
		if (SelectedBuildableActor != nullptr)
		{
			SelectedBuildableActor->SetHoverPreview(true);
		}
		return;
	}

	SetBuildState(ECampBuildState::Idle);
}

void UCampBuildComponent::RotatePreview(float YawDelta)
{
	if (BuildState != ECampBuildState::Placement && BuildState != ECampBuildState::Move && BuildState != ECampBuildState::Modify)
	{
		return;
	}

	CurrentBuildYaw = FMath::Fmod(CurrentBuildYaw + YawDelta, 360.0f);
	if (CurrentBuildYaw < 0.0f)
	{
		CurrentBuildYaw += 360.0f;
	}

	if (BuildState == ECampBuildState::Modify)
	{
		UpdateModifyPreviewAtSelectedBuildable();
	}
	else
	{
		UpdateBuildPreviewActor();
	}
}

void UCampBuildComponent::ChangeSelectedBuilding(int32 Direction)
{
	if (BuildState != ECampBuildState::Placement && BuildState != ECampBuildState::Modify)
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
	if (BuildState == ECampBuildState::Modify)
	{
		UpdateModifyPreviewAtSelectedBuildable();
	}
	else
	{
		UpdateBuildPreviewActor();
	}
}

void UCampBuildComponent::ChangeSelectedTier(int32 Direction)
{
	if (BuildState != ECampBuildState::Placement && BuildState != ECampBuildState::Modify)
	{
		return;
	}

	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent == nullptr)
	{
		return;
	}

	SelectionComponent->ChangeSelectedTier(Direction);
	if (BuildState == ECampBuildState::Modify)
	{
		DestroyBuildPreviewActor();
		SpawnBuildPreviewActor();
		UpdateModifyPreviewAtSelectedBuildable();
		return;
	}

	DestroyBuildPreviewActor();
	SpawnBuildPreviewActor();
	UpdateBuildPreviewActor();
}

void UCampBuildComponent::UpdateBuildTrace()
{
	if (bUseAutoPlacementLocation && ShouldUseAutoPlacementLocation())
	{
		UpdateAutoPlacementPreview();
		return;
	}

	bUseAutoPlacementLocation = false;

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

void UCampBuildComponent::UpdateExistingBuildTrace()
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

void UCampBuildComponent::SetBuildState(ECampBuildState NewBuildState)
{
	if (BuildState == NewBuildState)
	{
		return;
	}

	const bool bKeepSelectedBuildable = NewBuildState == ECampBuildState::Edit || NewBuildState == ECampBuildState::Move || NewBuildState == ECampBuildState::Modify;
	if (NewBuildState != ECampBuildState::Placement)
	{
		bUseAutoPlacementLocation = false;
	}
	ClearPlacementState();
	ClearHoveredBuildableActor();
	if (!bKeepSelectedBuildable)
	{
		ClearSelectedBuildableActor();
	}
	BuildState = NewBuildState;
	bHasCurrentTraceHit = false;
	bCanPlaceCurrentPreview = false;
	bSurfacePlacementBlocked = false;
	OnBuildStateChanged.Broadcast(BuildState);
}

void UCampBuildComponent::ClearPlacementState()
{
	DestroyBuildPreviewActor();
}

void UCampBuildComponent::ClearExistingBuildState()
{
	ClearHoveredBuildableActor();
	ClearSelectedBuildableActor();
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
		if (BuildState == ECampBuildState::Demolition)
		{
			HoveredBuildableActor->SetDemolitionPreview(true);
		}
		else if (BuildState == ECampBuildState::Edit)
		{
			HoveredBuildableActor->SetHoverPreview(true);
		}
	}
	OnBuildHoverChanged.Broadcast(HoveredBuildableActor != nullptr);
}

void UCampBuildComponent::ClearHoveredBuildableActor()
{
	if (HoveredBuildableActor != nullptr)
	{
		if (HoveredBuildableActor != SelectedBuildableActor)
		{
			HoveredBuildableActor->SetHoverPreview(false);
		}
		HoveredBuildableActor->SetDemolitionPreview(false);
		HoveredBuildableActor = nullptr;
		OnBuildHoverChanged.Broadcast(false);
	}
}

void UCampBuildComponent::ClearSelectedBuildableActor()
{
	if (SelectedBuildableActor != nullptr)
	{
		SelectedBuildableActor->SetHoverPreview(false);
		SelectedBuildableActor->SetDemolitionPreview(false);
	}
	SelectedBuildableActor = nullptr;
}

bool UCampBuildComponent::SelectCurrentDataForBuildable(ABuildableActor* BuildableActor)
{
	if (BuildableActor == nullptr)
	{
		return false;
	}

	UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (SelectionComponent == nullptr)
	{
		return false;
	}

	if (SelectionComponent->GetBuildingRowNames().IsEmpty())
	{
		SelectionComponent->LoadBuildingDataRows();
	}

	if (!BuildableActor->BuildingRowName.IsNone() && SelectionComponent->SelectBuildingByRowName(BuildableActor->BuildingRowName))
	{
		return true;
	}

	return !BuildableActor->BuildingTypeID.IsNone() && SelectionComponent->SelectTierForType(BuildableActor->BuildingTypeID, BuildableActor->Tier);
}

bool UCampBuildComponent::IsSelectedDataSameAsSelectedBuildable() const
{
	if (SelectedBuildableActor == nullptr)
	{
		return false;
	}

	const UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	const FBuildingDataRow* BuildingData = GetSelectedBuildingData();
	if (SelectionComponent == nullptr || BuildingData == nullptr)
	{
		return false;
	}

	if (!SelectedBuildableActor->BuildingRowName.IsNone())
	{
		return SelectionComponent->GetCurrentBuildingRowName() == SelectedBuildableActor->BuildingRowName;
	}

	return BuildingData->BuildingTypeID == SelectedBuildableActor->BuildingTypeID
		&& BuildingData->Tier == SelectedBuildableActor->Tier;
}

void UCampBuildComponent::PrepareSelectedBuildableForPreview(bool bKeepOriginHighlighted)
{
	if (SelectedBuildableActor == nullptr)
	{
		return;
	}

	SelectedBuildableActor->SetHoverPreview(false);
	SelectedBuildableActor->SetDemolitionPreview(false);
	if (bKeepOriginHighlighted)
	{
		SelectedBuildableActor->SetHoverPreview(true);
	}
	SelectedBuildableActor->SetActorHiddenInGame(!bKeepOriginHighlighted);
	SelectedBuildableActor->SetActorEnableCollision(false);
}

void UCampBuildComponent::RestoreSelectedBuildableAfterPreview()
{
	if (SelectedBuildableActor == nullptr)
	{
		return;
	}

	SelectedBuildableActor->SetHoverPreview(true);
	SelectedBuildableActor->SetActorHiddenInGame(false);
	SelectedBuildableActor->SetActorEnableCollision(true);
}

void UCampBuildComponent::UpdateModifyPreviewAtSelectedBuildable()
{
	if (SelectedBuildableActor == nullptr || CurrentPreviewActor == nullptr)
	{
		bHasCurrentTraceHit = false;
		bCanPlaceCurrentPreview = false;
		return;
	}

	const FVector SelectedLocation = SelectedBuildableActor->GetActorLocation();
	const float GroundZ = SelectedLocation.Z - SelectedBuildableActor->ActorHalfHeight;
	CurrentSnappedLocation = FVector(SelectedLocation.X, SelectedLocation.Y, GroundZ);
	bHasCurrentTraceHit = true;
	bSurfacePlacementBlocked = false;

	UpdatePlacementBoxTrace();
	UpdateBuildPreviewActor();
}

bool UCampBuildComponent::MoveSelectedBuildableToPreview()
{
	if (SelectedBuildableActor == nullptr || CurrentPreviewActor == nullptr || !bHasCurrentTraceHit || !bCanPlaceCurrentPreview)
	{
		return false;
	}

	SelectedBuildableActor->SetActorTransform(CurrentPreviewActor->GetActorTransform());
	RestoreSelectedBuildableAfterPreview();
	SetBuildState(ECampBuildState::Idle);
	return true;
}

bool UCampBuildComponent::ReplaceSelectedBuildableWithSelectedData()
{
	if (SelectedBuildableActor == nullptr || CurrentPreviewActor == nullptr || !bHasCurrentTraceHit || !bCanPlaceCurrentPreview)
	{
		return false;
	}

	ABuildableActor* NewBuildableActor = CurrentPreviewActor;
	CurrentPreviewActor = nullptr;
	NewBuildableActor->FinalizeBuild();

	SelectedBuildableActor->SetHoverPreview(false);
	SelectedBuildableActor->SetDemolitionPreview(false);
	SelectedBuildableActor->Destroy();
	SelectedBuildableActor = NewBuildableActor;
	SetBuildState(ECampBuildState::Edit);
	return true;
}

void UCampBuildComponent::ApplySelectedBuildingDataToActor(ABuildableActor* BuildableActor) const
{
	if (BuildableActor == nullptr)
	{
		return;
	}

	const FVector SelectedBuildSize = GetSelectedBuildSize();
	if (!SelectedBuildSize.IsNearlyZero())
	{
		BuildableActor->SetBuildSize(SelectedBuildSize);
	}

	if (const FBuildingDataRow* BuildingData = GetSelectedBuildingData())
	{
		BuildableActor->SetBuildingData(
			GetBuildingSelectionComponent() != nullptr ? GetBuildingSelectionComponent()->GetCurrentBuildingRowName() : NAME_None,
			BuildingData->BuildingTypeID,
			BuildingData->Tier);

		if (UStaticMesh* StaticMesh = BuildingData->StaticMesh.LoadSynchronous())
		{
			BuildableActor->SetBuildMesh(StaticMesh);
		}
	}
}

ABuildableActor* UCampBuildComponent::SpawnCompletedBuildActor(const FTransform& BuildTransform) const
{
	const TSubclassOf<ABuildableActor> SelectedBuildActorClass = GetSelectedBuildActorClass();
	UWorld* World = GetWorld();
	if (World == nullptr || SelectedBuildActorClass == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABuildableActor* BuiltActor = World->SpawnActor<ABuildableActor>(SelectedBuildActorClass, BuildTransform, SpawnParams);
	if (BuiltActor == nullptr)
	{
		return nullptr;
	}

	ApplySelectedBuildingDataToActor(BuiltActor);
	BuiltActor->FinalizeBuild();
	return BuiltActor;
}

AConstructionSiteActor* UCampBuildComponent::SpawnConstructionSiteActor(const FTransform& BuildTransform) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	TSubclassOf<AConstructionSiteActor> SiteClass = ConstructionSiteActorClass;
	if (SiteClass == nullptr)
	{
		SiteClass = AConstructionSiteActor::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AConstructionSiteActor* ConstructionSiteActor = World->SpawnActor<AConstructionSiteActor>(SiteClass, BuildTransform, SpawnParams);
	if (ConstructionSiteActor == nullptr)
	{
		return nullptr;
	}

	const FBuildingDataRow* BuildingData = GetSelectedBuildingData();
	const UBuildingSelectionComponent* SelectionComponent = GetBuildingSelectionComponent();
	if (BuildingData == nullptr || SelectionComponent == nullptr)
	{
		return ConstructionSiteActor;
	}

	UStaticMesh* StaticMesh = BuildingData->StaticMesh.LoadSynchronous();
	ConstructionSiteActor->InitializeConstructionSite(
		SelectionComponent->GetCurrentBuildingRowName(),
		BuildingData->BuildingTypeID,
		BuildingData->Tier,
		GetSelectedBuildSize(),
		GetSelectedBuildActorClass(),
		StaticMesh);

	return ConstructionSiteActor;
}

AActor* UCampBuildComponent::SpawnPlacedBuildActor(const FTransform& BuildTransform) const
{
	if (bUseConstructionSitePlacement)
	{
		return SpawnConstructionSiteActor(BuildTransform);
	}

	return SpawnCompletedBuildActor(BuildTransform);
}

void UCampBuildComponent::SetupAutoNextPlacementFromActor(const AActor* PlacedActor)
{
	if (!bAutoPlaceNextPreviewBesideBuiltActor || PlacedActor == nullptr || CurrentPreviewActor == nullptr)
	{
		UpdateBuildPreviewActor();
		return;
	}

	const FVector2D FootprintSize = GetGridAlignedFootprintSize();
	const FVector SideOffset = FRotator(0.0f, CurrentBuildYaw, 0.0f).RotateVector(FVector(0.0f, FootprintSize.Y, 0.0f));
	const FVector PlacedActorLocation = PlacedActor->GetActorLocation();
	const float GroundZ = PlacedActorLocation.Z - GetBuildActorHalfHeight();
	AutoPlacementLocation = FVector(PlacedActorLocation.X, PlacedActorLocation.Y, GroundZ) + SideOffset;

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = OwnerPawn != nullptr ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (PC != nullptr && PC->PlayerCameraManager != nullptr)
	{
		AutoPlacementCameraLocation = PC->PlayerCameraManager->GetCameraLocation();
		AutoPlacementCameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	}

	bUseAutoPlacementLocation = true;
	UpdateAutoPlacementPreview();
}

bool UCampBuildComponent::ShouldUseAutoPlacementLocation() const
{
	if (!bUseAutoPlacementLocation || BuildState != ECampBuildState::Placement || CurrentPreviewActor == nullptr)
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = OwnerPawn != nullptr ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (PC == nullptr || PC->PlayerCameraManager == nullptr)
	{
		return true;
	}

	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	const bool bCameraMoved = FVector::DistSquared(CameraLocation, AutoPlacementCameraLocation) > FMath::Square(5.0f);
	const bool bCameraRotated = FMath::Abs(FMath::FindDeltaAngleDegrees(CameraRotation.Yaw, AutoPlacementCameraRotation.Yaw)) > 1.0f
		|| FMath::Abs(FMath::FindDeltaAngleDegrees(CameraRotation.Pitch, AutoPlacementCameraRotation.Pitch)) > 1.0f;

	return !bCameraMoved && !bCameraRotated;
}

void UCampBuildComponent::UpdateAutoPlacementPreview()
{
	bHasCurrentTraceHit = true;
	CurrentTraceHit.ImpactPoint = AutoPlacementLocation;
	CurrentGridIndex = WorldLocationToGridIndex(AutoPlacementLocation);
	CurrentSnappedLocation = AutoPlacementLocation;

	UpdateSurfaceTraces();
	UpdatePlacementBoxTrace();
	UpdateBuildPreviewActor();
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
	if (SelectedBuildableActor != nullptr && (BuildState == ECampBuildState::Move || BuildState == ECampBuildState::Modify))
	{
		QueryParams.AddIgnoredActor(SelectedBuildableActor);
	}

	return World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, BuildTraceChannel, QueryParams);
}

bool UCampBuildComponent::IsBlockedSurfaceHit(const FHitResult& HitResult) const
{
	const AActor* HitActor = HitResult.GetActor();
	return HitActor != nullptr && HitActor != CurrentPreviewActor && HitActor != SelectedBuildableActor && HitActor->IsA<ABuildableActor>();
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
	if (SelectedBuildableActor != nullptr && (BuildState == ECampBuildState::Move || BuildState == ECampBuildState::Modify))
	{
		QueryParams.AddIgnoredActor(SelectedBuildableActor);
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
		ApplySelectedBuildingDataToActor(CurrentPreviewActor);
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
	if (BuildState == ECampBuildState::Modify && IsSelectedDataSameAsSelectedBuildable())
	{
		CurrentPreviewActor->SetPreviewHighlighted(true);
	}
	else
	{
		CurrentPreviewActor->SetPreviewHighlighted(false);
		CurrentPreviewActor->SetPlacementValid(bCanPlaceCurrentPreview);
	}
}
