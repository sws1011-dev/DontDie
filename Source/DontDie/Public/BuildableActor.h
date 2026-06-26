// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableActor.generated.h"

UCLASS()
class DONTDIE_API ABuildableActor : public AActor
{
	GENERATED_BODY()

public:
	ABuildableActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build|Mesh")
	class USceneComponent* SceneRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build|Mesh")
	class UStaticMeshComponent* StaticMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	class UMaterialInterface* ValidPreviewMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	class UMaterialInterface* InvalidPreviewMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	class UMaterialInterface* HoverPreviewMaterial = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Build|Data")
	FName BuildingRowName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Build|Data")
	FName BuildingTypeID = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Build|Data")
	int32 Tier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Size", meta = (ClampMin = "1.0"))
	FVector2D FootprintSize = FVector2D(40.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Size", meta = (ClampMin = "1.0"))
	float PlacementBoxHalfHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Size", meta = (ClampMin = "0.0"))
	float ActorHalfHeight = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetPreviewMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetPlacementValid(bool bIsValid);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetPreviewHighlighted(bool bHighlighted);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetDemolitionPreview(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetHoverPreview(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void FinalizeBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool IsPreviewMode() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildSize(const FVector& NewBuildSize);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildMesh(class UStaticMesh* NewMesh);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildingData(FName NewBuildingRowName, FName NewBuildingTypeID, int32 NewTier);

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	bool bPreviewMode = false;
	bool bDemolitionPreview = false;
	bool bHoverPreview = false;
	
	UPROPERTY()
	class UMaterialInstanceDynamic* RuntimeHoverPreviewMaterial = nullptr;

	TMap<class UMeshComponent*, TArray<class UMaterialInterface*>> OriginalMaterials;

	void CacheOriginalMaterials();
	void RestoreOriginalMaterials();
	void SetActorCollisionEnabled(bool bEnabled);
	class UMaterialInterface* GetHoverPreviewMaterial();
	void ApplyMaterial(class UMaterialInterface* Material);
	void UpdateStaticMeshSize();
};
