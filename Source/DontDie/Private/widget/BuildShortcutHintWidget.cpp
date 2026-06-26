// Fill out your copyright notice in the Description page of Project Settings.

#include "widget/BuildShortcutHintWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

void UBuildShortcutHintWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Txt_BuildMode == nullptr)
	{
		Txt_BuildMode = Cast<UTextBlock>(GetWidgetFromName(TEXT("Txt_BuildMode")));
	}
	if (Border_BuildMode == nullptr)
	{
		Border_BuildMode = Cast<UBorder>(GetWidgetFromName(TEXT("Border_BuildMode")));
	}
	if (ShortcutListPanel == nullptr)
	{
		ShortcutListPanel = Cast<UPanelWidget>(GetWidgetFromName(TEXT("ShortcutListPanel")));
	}
}

void UBuildShortcutHintWidget::RefreshForBuildState(ECampBuildState BuildState, bool bHasHoveredBuildable)
{
	if (Txt_BuildMode == nullptr)
	{
		Txt_BuildMode = Cast<UTextBlock>(GetWidgetFromName(TEXT("Txt_BuildMode")));
	}
	if (Border_BuildMode == nullptr)
	{
		Border_BuildMode = Cast<UBorder>(GetWidgetFromName(TEXT("Border_BuildMode")));
	}
	if (ShortcutListPanel == nullptr)
	{
		ShortcutListPanel = Cast<UPanelWidget>(GetWidgetFromName(TEXT("ShortcutListPanel")));
	}

	if (ShortcutListPanel == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildShortcutHintWidget: ShortcutListPanel is missing. Name a VerticalBox or ScrollBox 'ShortcutListPanel'."));
		OnBuildStateHintRefreshed(BuildState, bHasHoveredBuildable);
		return;
	}

	FText ModeText = FText::FromString(TEXT("건축 모드"));
	FLinearColor ModeColor = FLinearColor(0.78f, 0.82f, 0.86f, 0.85f);
	TArray<FText> HintTexts = {
		FText::FromString(TEXT("B 건축 모드 종료"))
	};

	switch (BuildState)
	{
	case ECampBuildState::Idle:
		ModeText = FText::FromString(TEXT("건축 모드"));
		ModeColor = FLinearColor(0.78f, 0.82f, 0.86f, 0.85f);
		HintTexts.Add(FText::FromString(TEXT("Tab 건물 목록")));
		HintTexts.Add(FText::FromString(TEXT("X 철거 모드")));
		if (bHasHoveredBuildable)
		{
			HintTexts.Add(FText::FromString(TEXT("C 건물 편집")));
		}
		break;
	case ECampBuildState::BuildList:
		ModeText = FText::FromString(TEXT("건물 목록"));
		ModeColor = FLinearColor(0.20f, 0.48f, 0.90f, 0.90f);
		HintTexts.Add(FText::FromString(TEXT("Tab 목록 닫기")));
		HintTexts.Add(FText::FromString(TEXT("좌클릭 건물 선택")));
		HintTexts.Add(FText::FromString(TEXT("우클릭 취소")));
		break;
	case ECampBuildState::Placement:
		ModeText = FText::FromString(TEXT("배치 중"));
		ModeColor = FLinearColor(0.16f, 0.72f, 0.38f, 0.90f);
		HintTexts.Add(FText::FromString(TEXT("좌클릭 건설")));
		HintTexts.Add(FText::FromString(TEXT("우클릭 건설 취소")));
		HintTexts.Add(FText::FromString(TEXT("휠 회전")));
		HintTexts.Add(FText::FromString(TEXT("Q/E 티어 변경")));
		HintTexts.Add(FText::FromString(TEXT("1/3 종류 변경")));
		break;
	case ECampBuildState::Edit:
		ModeText = FText::FromString(TEXT("편집 모드"));
		ModeColor = FLinearColor(0.15f, 0.65f, 0.78f, 0.90f);
		HintTexts.Add(FText::FromString(TEXT("X 건물 삭제")));
		HintTexts.Add(FText::FromString(TEXT("M 건물 이동")));
		HintTexts.Add(FText::FromString(TEXT("F 건물 수정")));
		HintTexts.Add(FText::FromString(TEXT("C 편집 모드 종료")));
		break;
	case ECampBuildState::Demolition:
		ModeText = FText::FromString(TEXT("철거 모드"));
		ModeColor = FLinearColor(0.88f, 0.16f, 0.16f, 0.90f);
		HintTexts.Add(FText::FromString(TEXT("좌클릭 철거")));
		HintTexts.Add(FText::FromString(TEXT("X/우클릭 철거 모드 종료")));
		break;
	case ECampBuildState::Move:
		ModeText = FText::FromString(TEXT("이동 중"));
		ModeColor = FLinearColor(0.94f, 0.68f, 0.18f, 0.90f);
		HintTexts.Add(FText::FromString(TEXT("좌클릭 이동 확정")));
		HintTexts.Add(FText::FromString(TEXT("우클릭 취소")));
		HintTexts.Add(FText::FromString(TEXT("휠 회전")));
		break;
	case ECampBuildState::Modify:
		ModeText = FText::FromString(TEXT("수정 중"));
		ModeColor = FLinearColor(0.58f, 0.34f, 0.86f, 0.90f);
		HintTexts.Add(FText::FromString(TEXT("좌클릭 수정 확정")));
		HintTexts.Add(FText::FromString(TEXT("우클릭 취소")));
		HintTexts.Add(FText::FromString(TEXT("Q/E 티어 변경")));
		HintTexts.Add(FText::FromString(TEXT("1/3 종류 변경")));
		HintTexts.Add(FText::FromString(TEXT("휠 회전")));
		break;
	default:
		break;
	}

	if (Txt_BuildMode != nullptr)
	{
		Txt_BuildMode->SetText(ModeText);
		Txt_BuildMode->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}

	if (Border_BuildMode != nullptr)
	{
		Border_BuildMode->SetBrushColor(ModeColor);
	}

	ShortcutListPanel->ClearChildren();
	for (const FText& HintText : HintTexts)
	{
		UTextBlock* TextBlock = WidgetTree != nullptr ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock == nullptr)
		{
			continue;
		}

		TextBlock->SetText(HintText);
		ShortcutListPanel->AddChild(TextBlock);
	}

	OnBuildStateHintRefreshed(BuildState, bHasHoveredBuildable);
}
