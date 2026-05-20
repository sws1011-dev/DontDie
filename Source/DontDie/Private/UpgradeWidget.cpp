#include "UpgradeWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UUpgradeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 위젯이 키보드/마우스 포커스를 받을 수 있도록 설정
    bIsFocusable = true;

    // 버튼 클릭 이벤트 바인딩
    if (Btn_OptionA) Btn_OptionA->OnClicked.AddDynamic(this, &UUpgradeWidget::OnOptionAClicked);
    if (Btn_OptionB) Btn_OptionB->OnClicked.AddDynamic(this, &UUpgradeWidget::OnOptionBClicked);
    if (Btn_OptionC) Btn_OptionC->OnClicked.AddDynamic(this, &UUpgradeWidget::OnOptionCClicked);
}

void UUpgradeWidget::SetupUpgradeOptions(const TArray<FUpgradeCardData>& Options)
{
    // 2중 방어벽: 넘어온 카드가 3개 미만이면 안전하게 탈출
    if (Options.Num() < 3)
    {
        UE_LOG(LogTemp, Error, TEXT("Upgrade Options count is less than 3! Count: %d"), Options.Num());
        RemoveFromParent();
        return;
    }

    // 기존 데이터 비우기
    CurrentCardTypes.Empty();

    // 1. Option A 세팅
    if (Text_TitleA) Text_TitleA->SetText(FText::FromString(Options[0].DisplayName));
    if (Text_DescA)  Text_DescA->SetText(FText::FromString(Options[0].Description));
    CurrentCardTypes.Add(Options[0].UpgradeType);

    // 2. Option B 세팅
    if (Text_TitleB) Text_TitleB->SetText(FText::FromString(Options[1].DisplayName));
    if (Text_DescB)  Text_DescB->SetText(FText::FromString(Options[1].Description));
    CurrentCardTypes.Add(Options[1].UpgradeType);

    // 3. Option C 세팅
    if (Text_TitleC) Text_TitleC->SetText(FText::FromString(Options[2].DisplayName));
    if (Text_DescC)  Text_DescC->SetText(FText::FromString(Options[2].Description));
    CurrentCardTypes.Add(Options[2].UpgradeType);
}

void UUpgradeWidget::OnOptionAClicked() { SelectUpgrade(0); }
void UUpgradeWidget::OnOptionBClicked() { SelectUpgrade(1); }
void UUpgradeWidget::OnOptionCClicked() { SelectUpgrade(2); }

void UUpgradeWidget::SelectUpgrade(int32 OptionIndex)
{
    // 클릭 시점에 인덱스 유효성 체크 및 로그 출력
    if (!CurrentCardTypes.IsValidIndex(OptionIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("SelectUpgrade: Invalid Index %d! CurrentCardTypes size: %d"), OptionIndex, CurrentCardTypes.Num());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("SelectUpgrade: Option %d selected. Type: %d"), OptionIndex, (int32)CurrentCardTypes[OptionIndex]);

    ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->ApplyUpgrade(CurrentCardTypes[OptionIndex]);
    }

    RemoveFromParent();
}