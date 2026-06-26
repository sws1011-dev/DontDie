// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampBuildComponent.h"
#include "player/BasePlayerCharacter.h"
#include "CampPlayerCharacter.generated.h"

UCLASS()
class DONTDIE_API ACampPlayerCharacter : public ABasePlayerCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACampPlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	class UCampBuildComponent* CampBuildComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	class UBuildingSelectionComponent* BuildingSelectionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping")
	class UInputMappingContext* ImcBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Default")
	class UInputAction* IaToggleBuildMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaConfirmBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaCancelBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaToggleBuildList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaRotateBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaChangeBuildType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaChangeBuildTier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaToggleBuildEditMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaDeleteSelectedBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaMoveSelectedBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaEditSelectedBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Interaction")
	class UInputAction* IaInteract;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionTraceDistance = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Build")
	TSubclassOf<class UBuildingListWidget> BuildingListWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Build")
	TSubclassOf<class UBuildShortcutHintWidget> BuildShortcutHintWidgetClass;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:
	void ToggleBuildMode();
	void SetBuildMode(bool bEnabled);
	void ConfirmBuild();
	void CancelBuild();
	void ToggleBuildList();
	void RotateBuild(const struct FInputActionValue& Value);
	void ChangeBuildType(const struct FInputActionValue& Value);
	void ChangeBuildTier(const struct FInputActionValue& Value);
	void ToggleBuildEditMode();
	void DeleteSelectedBuild();
	void MoveSelectedBuild();
	void EditSelectedBuild();
	void Interact();
	bool CompleteLookedAtConstructionSite() const;
	void AddBuildMappingContext();
	void RemoveBuildMappingContext();
	void ShowBuildingListWidget();
	void HideBuildingListWidget();
	void ShowBuildShortcutHintWidget();
	void HideBuildShortcutHintWidget();

	UFUNCTION()
	void HandleBuildStateChanged(ECampBuildState NewBuildState);

	UFUNCTION()
	void HandleBuildHoverChanged(bool bHasHoveredBuildable);

	void RefreshBuildShortcutHintWidget();

	UPROPERTY()
	class UBuildingListWidget* BuildingListWidgetInstance = nullptr;

	UPROPERTY()
	class UBuildShortcutHintWidget* BuildShortcutHintWidgetInstance = nullptr;
};
