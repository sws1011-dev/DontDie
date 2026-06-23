// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreviewActor.generated.h"

UCLASS()
class DONTDIE_API APreviewActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APreviewActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	class USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	class UStaticMeshComponent* PreviewMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preview")
	FLinearColor ValidColor = FLinearColor(0.0f, 1.0f, 0.2f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preview")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.35f);

	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewValid(bool bIsValid);

	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewSize(float TileSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;

	void ApplyPreviewColor(const FLinearColor& Color);
};
