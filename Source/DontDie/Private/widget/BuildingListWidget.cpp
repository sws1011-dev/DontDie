// Fill out your copyright notice in the Description page of Project Settings.

#include "widget/BuildingListWidget.h"

#include "BuildingDataRow.h"
#include "BuildingSelectionComponent.h"
#include "CampBuildComponent.h"
#include "Components/PanelWidget.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "widget/BuildingListEntryWidget.h"

void UBuildingListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshBuildingList();
}

void UBuildingListWidget::InitializeBuildingList(UCampBuildComponent* InBuildComponent, UBuildingSelectionComponent* InSelectionComponent)
{
	BuildComponent = InBuildComponent;
	SelectionComponent = InSelectionComponent;
	RefreshBuildingList();
}

void UBuildingListWidget::RefreshBuildingList()
{
	if (BuildingListPanel == nullptr)
	{
		BuildingListPanel = Cast<UPanelWidget>(GetWidgetFromName(TEXT("BuildingListPanel")));
	}

	if (BuildingListPanel == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingListWidget: BuildingListPanel is missing. Name a ScrollBox or VerticalBox 'BuildingListPanel'."));
		return;
	}

	if (SelectionComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingListWidget: SelectionComponent is null."));
		return;
	}

	if (SelectionComponent->BuildingDataTable == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingListWidget: BuildingDataTable is null."));
		return;
	}

	BuildingListPanel->ClearChildren();
	SelectionComponent->LoadBuildingDataRows();
	UE_LOG(LogTemp, Log, TEXT("BuildingListWidget: Refresh rows=%d entryClass=%s"),
		SelectionComponent->GetBuildingRowNames().Num(),
		BuildingEntryWidgetClass != nullptr ? *BuildingEntryWidgetClass->GetName() : TEXT("None"));

	if (BuildingEntryWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingListWidget: BuildingEntryWidgetClass is not assigned."));
		return;
	}

	for (const FName RowName : SelectionComponent->GetBuildingRowNames())
	{
		const FBuildingDataRow* BuildingData = SelectionComponent->BuildingDataTable->FindRow<FBuildingDataRow>(RowName, TEXT("BuildingListWidget::RefreshBuildingList"));
		if (BuildingData == nullptr)
		{
			continue;
		}

		UUserWidget* EntryUserWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), BuildingEntryWidgetClass);

		UBuildingListEntryWidget* EntryWidget = Cast<UBuildingListEntryWidget>(EntryUserWidget);
		if (EntryWidget == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("BuildingListWidget: Entry widget class must inherit BuildingListEntryWidget."));
			continue;
		}

		EntryWidget->SetupEntry(this, RowName, BuildingData->DisplayName, BuildingData->Tier);
		BuildingListPanel->AddChild(EntryWidget);
	}
}

void UBuildingListWidget::SelectBuilding(FName BuildingRowName)
{
	if (BuildComponent != nullptr && BuildComponent->SelectBuildingByRowName(BuildingRowName))
	{
		SetVisibility(ESlateVisibility::Collapsed);

		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			PlayerController->SetShowMouseCursor(false);
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
}
