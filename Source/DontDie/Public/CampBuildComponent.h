// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CampBuildComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DONTDIE_API UCampBuildComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UCampBuildComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Selection")
	class UBuildingSelectionComponent* BuildingSelectionComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Trace")
	float CameraTraceDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Trace")
	TEnumAsByte<ECollisionChannel> BuildTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Grid", meta = (ClampMin = "1.0"))
	float GridCellSize = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Grid")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement", meta = (ClampMin = "1.0"))
	float SurfaceTraceHeight = 550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement", meta = (ClampMin = "0.0"))
	float PlacementBoxGroundClearance = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement", meta = (ClampMin = "0.0"))
	float PlacementBoxExtentShrink = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement", meta = (ClampMin = "0.0"))
	float SurfaceHeightTolerance = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Data")
	class UDataTable* BuildingDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Rotation")
	float RotateStepDegrees = 10.0f;

	UFUNCTION(BlueprintCallable, Category = "Build|Selection")
	void SetBuildingSelectionComponent(class UBuildingSelectionComponent* NewBuildingSelectionComponent);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void ToggleBuildMode();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool IsBuildModeEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool HasCurrentTraceHit() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	FVector GetCurrentTraceHitLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	FIntPoint GetCurrentGridIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool CanPlaceCurrentPreview() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	void ToggleBuildEditMode();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildEditMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool IsBuildEditModeEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool DeleteSelectedBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool MoveSelectedBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool EditSelectedBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool ConfirmBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void RotatePreview(float YawDelta);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void ChangeSelectedBuilding(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void ChangeSelectedTier(int32 Direction);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class ABuildableActor* CurrentPreviewActor = nullptr;

	bool bBuildMode = false;
	bool bHasCurrentTraceHit = false;
	bool bCanPlaceCurrentPreview = false;
	bool bSurfacePlacementBlocked = false;
	bool bBuildEditMode = false;
	float CurrentBuildYaw = 0.0f;

	UPROPERTY()
	class ABuildableActor* HoveredBuildableActor = nullptr;

	UPROPERTY()
	class ABuildableActor* SelectedBuildableActor = nullptr;

	FHitResult CurrentTraceHit;
	FIntPoint CurrentGridIndex = FIntPoint::ZeroValue;
	FVector CurrentSnappedLocation = FVector::ZeroVector;

	void UpdateBuildTrace();
	bool TraceFromCamera(FHitResult& OutHit) const;
	void UpdateBuildEditTrace();
	void SelectHoveredBuildable();
	void SetHoveredBuildableActor(class ABuildableActor* NewHoveredBuildableActor);
	void ClearHoveredBuildableActor();
	void ClearSelectedBuildableActor();
	class UBuildingSelectionComponent* GetBuildingSelectionComponent();
	const class UBuildingSelectionComponent* GetBuildingSelectionComponent() const;
	void SyncLegacyBuildingDataTable();
	bool SelectInitialBuilding();
	const struct FBuildingDataRow* GetSelectedBuildingData() const;
	FVector GetSelectedBuildSize() const;
	TSubclassOf<class ABuildableActor> GetSelectedBuildActorClass() const;
	FVector2D GetBuildFootprintSize() const;
	FVector2D GetGridAlignedFootprintSize() const;
	FVector2D GetPlacementHalfExtent() const;
	float GetPlacementBoxHalfHeight() const;
	float GetBuildActorHalfHeight() const;
	FIntPoint WorldLocationToGridIndex(const FVector& WorldLocation) const;
	FVector GridIndexToWorldLocation(const FIntPoint& GridIndex) const;
	void UpdateSurfaceTraces();
	bool TraceSurfaceAtLocation(const FVector& TraceLocation, FHitResult& OutHit) const;
	bool IsBlockedSurfaceHit(const FHitResult& HitResult) const;
	bool IsUnsupportedSurfaceHit(bool bHit, const FHitResult& HitResult) const;
	void UpdatePlacementBoxTrace();
	void SpawnBuildPreviewActor();
	void DestroyBuildPreviewActor();
	void UpdateBuildPreviewActor();
};
