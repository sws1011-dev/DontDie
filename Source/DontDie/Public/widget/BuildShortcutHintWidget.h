// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CampBuildComponent.h"
#include "BuildShortcutHintWidget.generated.h"

class UPanelWidget;
class UBorder;
class UTextBlock;

UCLASS()
class DONTDIE_API UBuildShortcutHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void RefreshForBuildState(ECampBuildState BuildState, bool bHasHoveredBuildable);

protected:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Txt_BuildMode;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UBorder* Border_BuildMode;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UPanelWidget* ShortcutListPanel;

	UFUNCTION(BlueprintImplementableEvent, Category = "Build|UI")
	void OnBuildStateHintRefreshed(ECampBuildState BuildState, bool bHasHoveredBuildable);
};
