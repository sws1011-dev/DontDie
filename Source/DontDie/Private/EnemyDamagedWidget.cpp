// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyDamagedWidget.h"

#include "Components/TextBlock.h"

void UEnemyDamagedWidget::SetDamageText(float Damage)
{
	if (DamageText != nullptr)
	{
		FText DamageValue = FText::AsNumber(FMath::RoundToInt(Damage));
		DamageText->SetText(DamageValue);
	}
}
