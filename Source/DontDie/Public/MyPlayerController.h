// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UInputMappingContext* ImcPlayerInput;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UPauseWidget> PauseWidgetClass;

	UFUNCTION()
	void TogglePauseMenu();

	void ResumeGame();
	
protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	class UPauseWidget* CurrentPauseWidget;
};
