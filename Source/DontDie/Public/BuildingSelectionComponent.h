// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildingSelectionComponent.generated.h"

class ABuildableActor;
class UDataTable;

struct FBuildingDataRow;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DONTDIE_API UBuildingSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildingSelectionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Data")
	UDataTable* BuildingDataTable = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	void LoadBuildingDataRows();

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	bool SelectBuildingByIndex(int32 BuildingIndex);

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	bool SelectBuildingByRowName(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	bool SelectClosestTierForType(FName BuildingTypeID, int32 DesiredTier);

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	bool SelectTierForCurrentType(int32 Tier);

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	bool SelectTierForType(FName BuildingTypeID, int32 Tier);

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	void ChangeSelectedBuilding(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	void ChangeSelectedTier(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	int32 FindBuildingIndexByRowName(FName RowName) const;

	const FBuildingDataRow* GetSelectedBuildingData() const;
	FVector GetSelectedBuildSize() const;
	TSubclassOf<ABuildableActor> GetSelectedBuildActorClass() const;

	const TArray<FName>& GetBuildingRowNames() const;
	const TArray<FName>& GetBuildingTypeIDs() const;
	FName GetCurrentBuildingRowName() const;
	FName GetCurrentBuildingTypeID() const;
	int32 GetSelectedBuildingIndex() const;
	int32 GetCurrentTier() const;

private:
	TArray<FName> BuildingRowNames;
	TArray<FName> BuildingTypeIDs;
	FName CurrentBuildingRowName = NAME_None;
	FName CurrentBuildingTypeID = NAME_None;
	int32 SelectedBuildingIndex = INDEX_NONE;
	int32 CurrentTier = 1;
	FVector CurrentBuildSize = FVector::ZeroVector;
	TSubclassOf<ABuildableActor> CurrentBuildActorClass;

	void ResetSelection();
};
