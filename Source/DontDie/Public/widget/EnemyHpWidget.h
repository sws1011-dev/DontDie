// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHpWidget.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API UEnemyHpWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UProgressBar* HpBar;
	
	void UpdateHealthBar(float HealthPercent);
};
