// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PersistentUpgradeWidget.generated.h"

/**
 * 영구 업그레이드 시스템을 위한 위젯
 */
UCLASS()
class DONTDIE_API UPersistentUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 특정 키(예: "MaxHP", "BaseDamage")에 해당하는 업그레이드를 시도합니다.
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	bool TryUpgrade(FString UpgradeKey);

	// 블루프린트 버튼에서 바로 호출할 수 있는 전용 함수들
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeMaxHP() { TryUpgrade(TEXT("MaxHP")); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeMoveSpeed() { TryUpgrade(TEXT("MoveSpeed")); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeDamage() { TryUpgrade(TEXT("BaseDamage")); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeCrit() { TryUpgrade(TEXT("CritChance")); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeAmmo() { TryUpgrade(TEXT("MaxAmmo")); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeFireRate() { TryUpgrade(TEXT("FireRate")); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeLife() { TryUpgrade(TEXT("LifeCount")); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpgradeGoldMultiplier() { TryUpgrade(TEXT("CurrencyMultiplier")); }

	// 현재 업그레이드 레벨을 가져옵니다.
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	int32 GetLevel(FString UpgradeKey) const;

	// 업그레이드 비용을 계산합니다.
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	int32 GetCost(FString UpgradeKey) const;

	// UI 갱신을 위한 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Upgrade")
	void RefreshUI();

	// 모든 텍스트 정보를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void UpdateAllTexts();

	// 메인 메뉴로 돌아가는 함수
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void OnCloseClicked();

	// 이전 화면으로 돌아가기 위해 호출한 위젯을 저장
	void SetReturnWidget(UUserWidget* InWidget) { ReturnWidget = InWidget; }

private:
	UPROPERTY()
	class UUserWidget* ReturnWidget;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, Category = "UI")
	FName MenuLevelName = TEXT("MainMenu");

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 TotalGold;

	// 전체 골드 표시
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_TotalGold;

	// 레벨 표시 텍스트들 (이름 규칙: Txt_Lvl_항목명)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_MaxHP;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_MoveSpeed;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_Damage;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_Crit;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_Ammo;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_FireRate;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_Life;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Lvl_GoldMultiplier;

	// 비용 표시 텍스트들 (이름 규칙: Txt_Cost_항목명)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_MaxHP;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_MoveSpeed;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_Damage;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_Crit;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_Ammo;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_FireRate;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_Life;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* Txt_Cost_GoldMultiplier;

	// 디자인 창에서 버튼 이름을 아래와 같이 지으면 자동으로 연결됩니다.
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_MaxHP;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_MoveSpeed;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_Damage;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_Crit;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_Ammo;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_FireRate;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_Life;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_GoldMultiplier;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* Btn_Close;
};
