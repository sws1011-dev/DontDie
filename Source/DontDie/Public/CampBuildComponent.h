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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Trace")
	float CameraTraceDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Trace")
	TEnumAsByte<ECollisionChannel> BuildTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Debug")
	bool bDrawDebugTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Debug")
	bool bDrawDebugText = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Grid", meta = (ClampMin = "1.0"))
	float GridCellSize = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Grid")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement", meta = (ClampMin = "1.0"))
	float SurfaceTraceHeight = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement", meta = (ClampMin = "0.0"))
	float PlacementBoxGroundClearance = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Spawn")
	TSubclassOf<class ABuildableActor> BuildActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Rotation")
	float RotateStepDegrees = 10.0f;

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
	bool ConfirmBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void RotatePreview(float YawDelta);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class ABuildableActor* CurrentPreviewActor = nullptr;

	bool bBuildMode = false;
	bool bHasCurrentTraceHit = false;
	bool bCanPlaceCurrentPreview = false;
	float CurrentBuildYaw = 0.0f;

	FHitResult CurrentTraceHit;
	FIntPoint CurrentGridIndex = FIntPoint::ZeroValue;
	FVector CurrentSnappedLocation = FVector::ZeroVector;
	FString CurrentSurfaceDebugText;
	FString CurrentBoxTraceDebugText;

	void UpdateBuildTrace();
	bool TraceFromCamera(FHitResult& OutHit) const;
	FVector2D GetBuildFootprintSize() const;
	FVector2D GetPlacementHalfExtent() const;
	float GetPlacementBoxHalfHeight() const;
	float GetBuildActorHalfHeight() const;
	FIntPoint WorldLocationToGridIndex(const FVector& WorldLocation) const;
	FVector GridIndexToWorldLocation(const FIntPoint& GridIndex) const;
	void UpdateSurfaceTraces();
	bool TraceSurfaceAtLocation(const FVector& TraceLocation, FHitResult& OutHit) const;
	FString BuildSurfaceTraceDebugLine(const TCHAR* Label, bool bHit, const FHitResult& HitResult) const;
	void UpdatePlacementBoxTrace();
	void SpawnBuildPreviewActor();
	void DestroyBuildPreviewActor();
	void UpdateBuildPreviewActor();
	void DrawCurrentTraceDebug() const;
};
