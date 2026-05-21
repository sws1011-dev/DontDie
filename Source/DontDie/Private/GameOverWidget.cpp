// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOverWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "PersistentUpgradeWidget.h"
#include "Components/TextBlock.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RestartButton != nullptr)
	{
		RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
	}

	if (ShopButton != nullptr)
	{
		ShopButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnShopClicked);
	}

	if (Btn_MainMenu != nullptr)
	{
		Btn_MainMenu->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
	}
}

void UGameOverWidget::OnRestartClicked()
{
	// GetCurrentLevelName(..., true)를 사용하여 에디터용 접두사(UEDPIE_ 등)를 제거한 순수 레벨 이름을 가져옵니다.
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevelName));
}

void UGameOverWidget::OnShopClicked()
{
	if (UpgradeWidgetClass != nullptr)
	{
		UPersistentUpgradeWidget* UpgradeUI = CreateWidget<UPersistentUpgradeWidget>(GetWorld(), UpgradeWidgetClass);
		if (UpgradeUI != nullptr)
		{
			// 돌아올 위젯으로 자기 자신 설정
			UpgradeUI->SetReturnWidget(this);
			UpgradeUI->AddToViewport();

			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(UpgradeUI->TakeWidget());
				PC->SetInputMode(InputMode);
			}

			// 상점이 열리면 결과창은 숨김
			SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop Button Clicked, but UpgradeWidgetClass is not assigned!"));
	}
}

void UGameOverWidget::OnMainMenuClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), MenuLevelName);
}

void UGameOverWidget::SetupResults(int32 StageGold, int32 TotalGold)
{
	if (StageGoldText)
	{
		StageGoldText->SetText(FText::AsNumber(StageGold));
	}

	if (TotalGoldText)
	{
		TotalGoldText->SetText(FText::AsNumber(TotalGold));
	}
}
