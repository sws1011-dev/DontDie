// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	class UButton* RestartButton;
	
	UFUNCTION()
	void OnRestartClicked();
	
	UPROPERTY(meta = (BindWidget))
	class UButton* ShopButton;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> UpgradeWidgetClass;

	UFUNCTION()
	void OnShopClicked();

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_MainMenu;

	UFUNCTION()
	void OnMainMenuClicked();

	UPROPERTY(EditAnywhere, Category = "UI")
	FName MenuLevelName = TEXT("MainMenu");

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StageGoldText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TotalGoldText;

	void SetupResults(int32 StageGold, int32 TotalGold);
};
