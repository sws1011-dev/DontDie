// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHudWidget.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API UPlayerHudWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateLifeText(int32 CurrentLife);
	
	UPROPERTY(meta =(BindWidget))
	class UTextBlock* LifeText;
	
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* WaveProgressBar;
	
	// 퍼센트를 업데이트하는 함수 추가
	void UpdateWaveProgress(float Percent);
};
