// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildableActor.h"
#include "ConstructionSiteActor.generated.h"

class UMaterialInterface;
class UStaticMesh;

UCLASS()
class DONTDIE_API AConstructionSiteActor : public ABuildableActor
{
	GENERATED_BODY()

public:
	AConstructionSiteActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build|Construction")
	class UBoxComponent* InteractionBoxComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Construction")
	UMaterialInterface* ConstructionSiteMaterial = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Build|Construction")
	TSubclassOf<ABuildableActor> TargetBuildActorClass;

	UFUNCTION(BlueprintCallable, Category = "Build|Construction")
	void InitializeConstructionSite(
		FName InBuildingRowName,
		FName InBuildingTypeID,
		int32 InTier,
		const FVector& InBuildSize,
		TSubclassOf<ABuildableActor> InTargetBuildActorClass,
		UStaticMesh* InStaticMesh);

	UFUNCTION(BlueprintCallable, Category = "Build|Construction")
	ABuildableActor* CompleteConstruction();

protected:
	virtual void BeginPlay() override;

private:
	void UpdateConstructionCollision();
	void ApplyConstructionMaterial();
};
