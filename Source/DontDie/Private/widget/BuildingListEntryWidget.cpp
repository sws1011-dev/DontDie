// Fill out your copyright notice in the Description page of Project Settings.

#include "widget/BuildingListEntryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "widget/BuildingListWidget.h"

void UBuildingListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select != nullptr && !Btn_Select->OnClicked.IsAlreadyBound(this, &UBuildingListEntryWidget::OnSelectClicked))
	{
		Btn_Select->OnClicked.AddDynamic(this, &UBuildingListEntryWidget::OnSelectClicked);
	}
}

void UBuildingListEntryWidget::SetupEntry(UBuildingListWidget* InOwnerListWidget, FName InBuildingRowName, const FText& InDisplayName, int32 InTier)
{
	OwnerListWidget = InOwnerListWidget;
	BuildingRowName = InBuildingRowName;

	if (Txt_Name != nullptr)
	{
		Txt_Name->SetText(InDisplayName);
	}

	if (Txt_Tier != nullptr)
	{
		Txt_Tier->SetText(FText::AsNumber(InTier));
	}
}

void UBuildingListEntryWidget::OnSelectClicked()
{
	if (OwnerListWidget != nullptr)
	{
		OwnerListWidget->SelectBuilding(BuildingRowName);
	}
}
