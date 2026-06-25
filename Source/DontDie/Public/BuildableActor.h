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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	class UMaterialInterface* ValidPreviewMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	class UMaterialInterface* InvalidPreviewMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	class UMaterialInterface* BuiltMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Size", meta = (ClampMin = "1.0"))
	FVector2D FootprintSize = FVector2D(400.0f, 400.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Size", meta = (ClampMin = "1.0"))
	float PlacementBoxHalfHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Size", meta = (ClampMin = "0.0"))
	float ActorHalfHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Size", meta = (ClampMin = "0.0"))
	float PreviewHalfHeight = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetPreviewMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetPlacementValid(bool bIsValid);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void FinalizeBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool IsPreviewMode() const;

private:
	bool bPreviewMode = false;
	TMap<class UMeshComponent*, TArray<class UMaterialInterface*>> OriginalMaterials;

	void CacheOriginalMaterials();
	void RestoreOriginalMaterials();
	void SetActorCollisionEnabled(bool bEnabled);
	void ApplyMaterial(class UMaterialInterface* Material);
};
