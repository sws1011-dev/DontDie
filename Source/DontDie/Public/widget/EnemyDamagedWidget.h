// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyDamagedWidget.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API UEnemyDamagedWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* DamageText;
	
	void SetDamageText(float Damage);
};
