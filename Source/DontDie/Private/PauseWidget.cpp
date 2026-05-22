#include "PauseWidget.h"
#include "MyPlayerController.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume) Btn_Resume->OnClicked.AddDynamic(this, &UPauseWidget::OnResumeClicked);
	if (Btn_MainMenu) Btn_MainMenu->OnClicked.AddDynamic(this, &UPauseWidget::OnMainMenuClicked);
	if (Btn_Quit) Btn_Quit->OnClicked.AddDynamic(this, &UPauseWidget::OnQuitClicked);
}

void UPauseWidget::OnResumeClicked()
{
	AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		PC->ResumeGame();
	}
	else
	{
		// Fallback
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		RemoveFromParent();
	}
}

void UPauseWidget::OnMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}

void UPauseWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
}
