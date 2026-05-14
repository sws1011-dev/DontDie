// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "EnhancedInputSubsystems.h"

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
