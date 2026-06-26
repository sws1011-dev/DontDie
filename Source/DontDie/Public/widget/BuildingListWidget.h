// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingListWidget.generated.h"

class UBuildingSelectionComponent;
class UCampBuildComponent;
class UPanelWidget;
class UUserWidget;

UCLASS()
class DONTDIE_API UBuildingListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void InitializeBuildingList(UCampBuildComponent* InBuildComponent, UBuildingSelectionComponent* InSelectionComponent);

	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void RefreshBuildingList();

	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void SelectBuilding(FName BuildingRowName);

protected:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UPanelWidget* BuildingListPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|UI")
	TSubclassOf<UUserWidget> BuildingEntryWidgetClass;

private:
	UPROPERTY()
	UCampBuildComponent* BuildComponent = nullptr;

	UPROPERTY()
	UBuildingSelectionComponent* SelectionComponent = nullptr;
};
