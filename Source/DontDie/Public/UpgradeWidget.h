// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DontDieGameModeBase.h" // FUpgradeCardData, EUpgradeType 사용을 위해 포함
#include "UpgradeWidget.generated.h"

UCLASS()
class DONTDIE_API UUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 위젯이 생성되고 초기화될 때 호출되는 함수 (이벤트 그래프의 Construct 단계)
	virtual void NativeConstruct() override;

public:
	// GameMode에서 뽑아준 3개의 옵션 데이터를 받아와 UI에 세팅하는 함수
	void SetupUpgradeOptions(const TArray<FUpgradeCardData>& Options);

protected:
	// 블루프린트의 UI 에셋과 C++ 변수를 이름 기반으로 강제 연동합니다.
	// (블루프린트에서 버튼 이름을 'Btn_OptionA'로 정확히 지어줘야 합니다.)
    
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_OptionA;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_TitleA;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_DescA;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_OptionB;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_TitleB;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_DescB;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_OptionC;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_TitleC;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_DescC;

private:
	// 각 버튼이 클릭되었을 때 어떤 업그레이드를 실행할지 저장해두는 임시 풀
	TArray<EUpgradeType> CurrentCardTypes;

	// 버튼 클릭 시 호출될 내부 함수들
	UFUNCTION() void OnOptionAClicked();
	UFUNCTION() void OnOptionBClicked();
	UFUNCTION() void OnOptionCClicked();

	// 실제 GameMode에게 선택을 전달하고 자신을 파괴하는 공통 로직
	void SelectUpgrade(int32 OptionIndex);
};