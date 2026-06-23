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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build")
	float TraceDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build")
	bool bDrawDebugTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	TSubclassOf<class APreviewActor> PreviewActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	float PreviewTileSize = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	float PreviewZOffset = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "Build")
	void ToggleBuildMode();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Build")
	bool IsBuildModeEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Build")
	class ACampGridTile* GetCurrentTile() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class ACampGridTile* CurrentTile;

	UPROPERTY()
	class APreviewActor* PreviewActor;

	bool bBuildMode = false;

	void UpdateTileTrace();
	void SetCurrentTile(class ACampGridTile* NewTile);
	void SpawnPreviewActor();
	void DestroyPreviewActor();
	void UpdatePreviewActor();
};
