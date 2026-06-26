// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingListEntryWidget.generated.h"

class UBuildingListWidget;
class UButton;
class UTextBlock;

UCLASS()
class DONTDIE_API UBuildingListEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetupEntry(UBuildingListWidget* InOwnerListWidget, FName InBuildingRowName, const FText& InDisplayName, int32 InTier);

protected:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UButton* Btn_Select;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Txt_Name;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Txt_Tier;

private:
	UPROPERTY()
	UBuildingListWidget* OwnerListWidget = nullptr;

	FName BuildingRowName = NAME_None;

	UFUNCTION()
	void OnSelectClicked();
};
