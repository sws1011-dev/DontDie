// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildingSelectionComponent.h"

#include "BuildableActor.h"
#include "BuildingDataRow.h"
#include "Engine/DataTable.h"

UBuildingSelectionComponent::UBuildingSelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBuildingSelectionComponent::LoadBuildingDataRows()
{
	BuildingRowNames.Empty();
	BuildingTypeIDs.Empty();

	if (BuildingDataTable == nullptr)
	{
		ResetSelection();
		return;
	}

	BuildingRowNames = BuildingDataTable->GetRowNames();

	for (const FName RowName : BuildingRowNames)
	{
		const FBuildingDataRow* BuildingData = BuildingDataTable->FindRow<FBuildingDataRow>(RowName, TEXT("BuildingSelectionComponent::LoadBuildingDataRows"));
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
		ResetSelection();
	}
}

bool UBuildingSelectionComponent::SelectBuildingByIndex(int32 BuildingIndex)
{
	if (BuildingDataTable == nullptr || !BuildingRowNames.IsValidIndex(BuildingIndex))
	{
		return false;
	}

	const FName RowName = BuildingRowNames[BuildingIndex];
	const FBuildingDataRow* BuildingData = BuildingDataTable->FindRow<FBuildingDataRow>(RowName, TEXT("BuildingSelectionComponent::SelectBuildingByIndex"));
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
	CurrentBuildActorClass = LoadedBuildActorClass;
	CurrentBuildSize = FVector(
		FMath::Max(BuildingData->SizeX, 1.0f),
		FMath::Max(BuildingData->SizeY, 1.0f),
		FMath::Max(BuildingData->SizeZ, 1.0f));

	return true;
}

bool UBuildingSelectionComponent::SelectBuildingByRowName(FName RowName)
{
	const int32 BuildingIndex = FindBuildingIndexByRowName(RowName);
	if (BuildingIndex == INDEX_NONE)
	{
		return false;
	}

	return SelectBuildingByIndex(BuildingIndex);
}

bool UBuildingSelectionComponent::SelectClosestTierForType(FName BuildingTypeID, int32 DesiredTier)
{
	int32 BestIndex = INDEX_NONE;
	int32 BestTierDistance = MAX_int32;
	int32 BestTier = MAX_int32;

	for (int32 Index = 0; Index < BuildingRowNames.Num(); ++Index)
	{
		const FBuildingDataRow* BuildingData = BuildingDataTable != nullptr
			? BuildingDataTable->FindRow<FBuildingDataRow>(BuildingRowNames[Index], TEXT("BuildingSelectionComponent::SelectClosestTierForType"))
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

bool UBuildingSelectionComponent::SelectTierForCurrentType(int32 Tier)
{
	return SelectTierForType(CurrentBuildingTypeID, Tier);
}

bool UBuildingSelectionComponent::SelectTierForType(FName BuildingTypeID, int32 Tier)
{
	if (Tier < 1 || BuildingTypeID.IsNone())
	{
		return false;
	}

	for (int32 Index = 0; Index < BuildingRowNames.Num(); ++Index)
	{
		const FBuildingDataRow* BuildingData = BuildingDataTable != nullptr
			? BuildingDataTable->FindRow<FBuildingDataRow>(BuildingRowNames[Index], TEXT("BuildingSelectionComponent::SelectTierForType"))
			: nullptr;
		if (BuildingData != nullptr && BuildingData->BuildingTypeID == BuildingTypeID && BuildingData->Tier == Tier)
		{
			return SelectBuildingByIndex(Index);
		}
	}

	return false;
}

void UBuildingSelectionComponent::ChangeSelectedBuilding(int32 Direction)
{
	if (Direction == 0)
	{
		return;
	}

	if (BuildingRowNames.IsEmpty())
	{
		LoadBuildingDataRows();
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

void UBuildingSelectionComponent::ChangeSelectedTier(int32 Direction)
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

int32 UBuildingSelectionComponent::FindBuildingIndexByRowName(FName RowName) const
{
	return BuildingRowNames.IndexOfByKey(RowName);
}

const FBuildingDataRow* UBuildingSelectionComponent::GetSelectedBuildingData() const
{
	if (BuildingDataTable == nullptr || CurrentBuildingRowName.IsNone())
	{
		return nullptr;
	}

	return BuildingDataTable->FindRow<FBuildingDataRow>(CurrentBuildingRowName, TEXT("BuildingSelectionComponent::GetSelectedBuildingData"));
}

FVector UBuildingSelectionComponent::GetSelectedBuildSize() const
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

TSubclassOf<ABuildableActor> UBuildingSelectionComponent::GetSelectedBuildActorClass() const
{
	return CurrentBuildActorClass;
}

const TArray<FName>& UBuildingSelectionComponent::GetBuildingRowNames() const
{
	return BuildingRowNames;
}

const TArray<FName>& UBuildingSelectionComponent::GetBuildingTypeIDs() const
{
	return BuildingTypeIDs;
}

FName UBuildingSelectionComponent::GetCurrentBuildingRowName() const
{
	return CurrentBuildingRowName;
}

FName UBuildingSelectionComponent::GetCurrentBuildingTypeID() const
{
	return CurrentBuildingTypeID;
}

int32 UBuildingSelectionComponent::GetSelectedBuildingIndex() const
{
	return SelectedBuildingIndex;
}

int32 UBuildingSelectionComponent::GetCurrentTier() const
{
	return CurrentTier;
}

void UBuildingSelectionComponent::ResetSelection()
{
	SelectedBuildingIndex = INDEX_NONE;
	CurrentBuildingRowName = NAME_None;
	CurrentBuildingTypeID = NAME_None;
	CurrentTier = 1;
	CurrentBuildSize = FVector::ZeroVector;
	CurrentBuildActorClass = nullptr;
}
