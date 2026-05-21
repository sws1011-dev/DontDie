// Fill out your copyright notice in the Description page of Project Settings.


#include "PersistentUpgradeWidget.h"
#include "DontDieGameModeBase.h"
#include "DontDieSaveGame.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

#include "Components/TextBlock.h"

void UPersistentUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 자동 바인딩 로직
	if (Btn_MaxHP) Btn_MaxHP->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeMaxHP);
	if (Btn_MoveSpeed) Btn_MoveSpeed->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeMoveSpeed);
	if (Btn_Damage) Btn_Damage->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeDamage);
	if (Btn_Crit) Btn_Crit->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeCrit);
	if (Btn_Ammo) Btn_Ammo->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeAmmo);
	if (Btn_FireRate) Btn_FireRate->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeFireRate);
	if (Btn_Life) Btn_Life->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeLife);
	if (Btn_GoldMultiplier) Btn_GoldMultiplier->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::UpgradeGoldMultiplier);

	if (Btn_Close) Btn_Close->OnClicked.AddDynamic(this, &UPersistentUpgradeWidget::OnCloseClicked);

	// 1. 세이브 파일에서 직접 골드 정보 로드 (메인 메뉴 대응)
	FString SaveSlotName = TEXT("DontDieSaveSlot");
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UDontDieSaveGame* SaveGameInstance = Cast<UDontDieSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		if (SaveGameInstance)
		{
			TotalGold = SaveGameInstance->TotalGold;
		}
	}
	else
	{
		TotalGold = 0;
	}

	// 2. 인게임 중이라면 게임 모드와 데이터 동기화
	ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		TotalGold = GM->TotalGold;
	}
	
	UpdateAllTexts();
	RefreshUI();
}

void UPersistentUpgradeWidget::OnCloseClicked()
{
	if (ReturnWidget)
	{
		// 1. 이전 화면을 다시 보이게 함
		ReturnWidget->SetVisibility(ESlateVisibility::Visible);

		// 2. 포커스를 이전 화면으로 돌려줌
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ReturnWidget->TakeWidget());
			PC->SetInputMode(InputMode);
		}

		// 3. 상점 위젯만 제거
		RemoveFromParent();
	}
	else
	{
		// 백업 로직: 메인 메뉴 레벨로 이동
		UGameplayStatics::OpenLevel(GetWorld(), MenuLevelName);
	}
}

bool UPersistentUpgradeWidget::TryUpgrade(FString UpgradeKey)
{
	// 1. 현재 보유 골드 확인
	int32 Cost = GetCost(UpgradeKey);
	if (TotalGold < Cost) return false;

	// 2. 데이터 업데이트 및 세이브
	FString SaveSlotName = TEXT("DontDieSaveSlot");
	UDontDieSaveGame* SaveGameInstance = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		SaveGameInstance = Cast<UDontDieSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}
	
	if (!SaveGameInstance)
	{
		SaveGameInstance = Cast<UDontDieSaveGame>(UGameplayStatics::CreateSaveGameObject(UDontDieSaveGame::StaticClass()));
	}

	if (SaveGameInstance)
	{
		int32 CurrentLevel = SaveGameInstance->UpgradeLevels.Contains(UpgradeKey) ? SaveGameInstance->UpgradeLevels[UpgradeKey] : 0;
		SaveGameInstance->UpgradeLevels.Add(UpgradeKey, CurrentLevel + 1);
		
		TotalGold -= Cost;
		SaveGameInstance->TotalGold = TotalGold;
		
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0);
		
		// 3. 인게임 중이라면 게임 모드 데이터도 즉시 동기화
		ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GM)
		{
			GM->TotalGold = TotalGold;
			GM->UpgradeLevels = SaveGameInstance->UpgradeLevels;
		}

		UpdateAllTexts();
		RefreshUI();
		return true;
	}

	return false;
}

int32 UPersistentUpgradeWidget::GetLevel(FString UpgradeKey) const
{
	FString SaveSlotName = TEXT("DontDieSaveSlot");
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UDontDieSaveGame* SaveGameInstance = Cast<UDontDieSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		if (SaveGameInstance && SaveGameInstance->UpgradeLevels.Contains(UpgradeKey))
		{
			return SaveGameInstance->UpgradeLevels[UpgradeKey];
		}
	}
	return 0;
}

int32 UPersistentUpgradeWidget::GetCost(FString UpgradeKey) const
{
	int32 Level = GetLevel(UpgradeKey);
	// 기본 비용 1000, 레벨당 500 증가 (원하시는 대로 수정 가능)
	return 1000 + (Level * 500);
}

void UPersistentUpgradeWidget::UpdateAllTexts()
{
	if (Txt_TotalGold) Txt_TotalGold->SetText(FText::AsNumber(TotalGold));

	auto UpdateStatText = [this](UTextBlock* LvlTxt, UTextBlock* CostTxt, FString Key)
	{
		if (LvlTxt) LvlTxt->SetText(FText::Format(FText::FromString(TEXT("Lv. {0}")), FText::AsNumber(GetLevel(Key))));
		if (CostTxt) CostTxt->SetText(FText::AsNumber(GetCost(Key)));
	};

	UpdateStatText(Txt_Lvl_MaxHP, Txt_Cost_MaxHP, TEXT("MaxHP"));
	UpdateStatText(Txt_Lvl_MoveSpeed, Txt_Cost_MoveSpeed, TEXT("MoveSpeed"));
	UpdateStatText(Txt_Lvl_Damage, Txt_Cost_Damage, TEXT("BaseDamage"));
	UpdateStatText(Txt_Lvl_Crit, Txt_Cost_Crit, TEXT("CritChance"));
	UpdateStatText(Txt_Lvl_Ammo, Txt_Cost_Ammo, TEXT("MaxAmmo"));
	UpdateStatText(Txt_Lvl_FireRate, Txt_Cost_FireRate, TEXT("FireRate"));
	UpdateStatText(Txt_Lvl_Life, Txt_Cost_Life, TEXT("LifeCount"));
	UpdateStatText(Txt_Lvl_GoldMultiplier, Txt_Cost_GoldMultiplier, TEXT("CurrencyMultiplier"));
}
