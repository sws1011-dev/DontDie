// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "PauseWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 커서 설정
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);

	// IMC 연결
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(ImcPlayerInput, 0);
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		// ESC 키 바인딩 (일시정지 중에도 작동하도록 설정)
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AMyPlayerController::TogglePauseMenu).bExecuteWhenPaused = true;
	}
}

void AMyPlayerController::TogglePauseMenu()
{
	// 1. 이미 일시정지 위젯이 떠 있다면 닫기
	if (CurrentPauseWidget && CurrentPauseWidget->IsInViewport())
	{
		ResumeGame();
		return;
	}

	// 2. 다른 사유(업그레이드 등)로 이미 게임이 일시정지 중이면 무시 (중복 UI 방지)
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		return;
	}

	// 3. 일시정지 메뉴 생성 및 표시
	if (PauseWidgetClass)
	{
		CurrentPauseWidget = CreateWidget<UPauseWidget>(this, PauseWidgetClass);
		if (CurrentPauseWidget)
		{
			CurrentPauseWidget->AddToViewport();
			UGameplayStatics::SetGamePaused(GetWorld(), true);

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(CurrentPauseWidget->TakeWidget());
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void AMyPlayerController::ResumeGame()
{
	if (CurrentPauseWidget)
	{
		CurrentPauseWidget->RemoveFromParent();
		CurrentPauseWidget = nullptr;
	}

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}
