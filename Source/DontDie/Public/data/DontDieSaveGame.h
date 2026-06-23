// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DontDieSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API UDontDieSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "SaveData")
	int32 TotalGold = 0;

	UPROPERTY(EditAnywhere, Category = "SaveData")
	TMap<FString, int32> UpgradeLevels;
};
