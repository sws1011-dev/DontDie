// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 게임 메인 메뉴 위젯
 */
UCLASS()
class DONTDIE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 버튼 바인딩 (디자인 창에서 이름을Btn_...으로 맞추세요)
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Start;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Upgrade;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Quit;

	// 연결할 업그레이드 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UPersistentUpgradeWidget> UpgradeWidgetClass;

	// 시작 시 이동할 맵 이름
	UPROPERTY(EditAnywhere, Category = "Game")
	FName PlayLevelName = TEXT("DontDie");

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnUpgradeClicked();

	UFUNCTION()
	void OnQuitClicked();
};
