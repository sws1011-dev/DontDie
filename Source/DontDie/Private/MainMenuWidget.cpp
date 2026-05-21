// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "PersistentUpgradeWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Start)
	{
		Btn_Start->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
	}

	if (Btn_Upgrade)
	{
		Btn_Upgrade->OnClicked.AddDynamic(this, &UMainMenuWidget::OnUpgradeClicked);
	}

	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}

	// 메뉴 화면에서는 마우스 커서 표시 및 입력 모드 설정
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}
}

void UMainMenuWidget::OnStartClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), PlayLevelName);
}

void UMainMenuWidget::OnUpgradeClicked()
{
	if (UpgradeWidgetClass != nullptr)
	{
		UPersistentUpgradeWidget* UpgradeUI = CreateWidget<UPersistentUpgradeWidget>(GetWorld(), UpgradeWidgetClass);
		if (UpgradeUI != nullptr)
		{
			// 돌아올 위젯으로 자기 자신 설정
			UpgradeUI->SetReturnWidget(this);
			UpgradeUI->AddToViewport();
			
			// 새 위젯에 포커스 주기
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(UpgradeUI->TakeWidget());
				PC->SetInputMode(InputMode);
			}

			// 상점이 열리면 메인 메뉴는 숨김
			SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: UpgradeWidgetClass is not assigned!"));
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
}
