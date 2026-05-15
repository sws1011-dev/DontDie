// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOverWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RestartButton != nullptr)
	{
		RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
	}
}

void UGameOverWidget::OnRestartClicked()
{
	FString CurrentMapName = GetWorld()->GetMapName();
	UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentMapName));
}
