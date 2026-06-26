// Fill out your copyright notice in the Description page of Project Settings.


#include "player/CampPlayerCharacter.h"

#include "BuildingSelectionComponent.h"
#include "CampBuildComponent.h"
#include "ConstructionSiteActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "UObject/ConstructorHelpers.h"
#include "widget/BuildShortcutHintWidget.h"
#include "widget/BuildingListWidget.h"

ACampPlayerCharacter::ACampPlayerCharacter()
{
	BuildingSelectionComponent = CreateDefaultSubobject<UBuildingSelectionComponent>(TEXT("BuildingSelectionComponent"));
	CampBuildComponent = CreateDefaultSubobject<UCampBuildComponent>(TEXT("CampBuildComponent"));
	CampBuildComponent->SetBuildingSelectionComponent(BuildingSelectionComponent);

	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleBuildListActionFinder(TEXT("/Game/Inputs/IA_ToggleBuildList.IA_ToggleBuildList"));
	if (ToggleBuildListActionFinder.Succeeded())
	{
		IaToggleBuildList = ToggleBuildListActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionFinder(TEXT("/Game/Inputs/IA_Interact.IA_Interact"));
	if (InteractActionFinder.Succeeded())
	{
		IaInteract = InteractActionFinder.Object;
	}

}

void ACampPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CampBuildComponent != nullptr)
	{
		CampBuildComponent->OnBuildStateChanged.AddDynamic(this, &ACampPlayerCharacter::HandleBuildStateChanged);
		CampBuildComponent->OnBuildHoverChanged.AddDynamic(this, &ACampPlayerCharacter::HandleBuildHoverChanged);
	}
}

void ACampPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		return;
	}

	if (IaToggleBuildMode != nullptr)
	{
		EnhancedInputComponent->BindAction(IaToggleBuildMode, ETriggerEvent::Started, this, &ACampPlayerCharacter::ToggleBuildMode);
	}

	if (IaConfirmBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaConfirmBuild, ETriggerEvent::Started, this, &ACampPlayerCharacter::ConfirmBuild);
	}

	if (IaCancelBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaCancelBuild, ETriggerEvent::Started, this, &ACampPlayerCharacter::CancelBuild);
	}

	if (IaToggleBuildList != nullptr)
	{
		EnhancedInputComponent->BindAction(IaToggleBuildList, ETriggerEvent::Started, this, &ACampPlayerCharacter::ToggleBuildList);
	}

	if (IaRotateBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaRotateBuild, ETriggerEvent::Started, this, &ACampPlayerCharacter::RotateBuild);
	}

	if (IaChangeBuildType != nullptr)
	{
		EnhancedInputComponent->BindAction(IaChangeBuildType, ETriggerEvent::Started, this, &ACampPlayerCharacter::ChangeBuildType);
	}

	if (IaChangeBuildTier != nullptr)
	{
		EnhancedInputComponent->BindAction(IaChangeBuildTier, ETriggerEvent::Started, this, &ACampPlayerCharacter::ChangeBuildTier);
	}

	if (IaToggleBuildEditMode != nullptr)
	{
		EnhancedInputComponent->BindAction(IaToggleBuildEditMode, ETriggerEvent::Started, this, &ACampPlayerCharacter::ToggleBuildEditMode);
	}

	if (IaDeleteSelectedBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaDeleteSelectedBuild, ETriggerEvent::Started, this, &ACampPlayerCharacter::DeleteSelectedBuild);
	}

	if (IaMoveSelectedBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaMoveSelectedBuild, ETriggerEvent::Started, this, &ACampPlayerCharacter::MoveSelectedBuild);
	}

	if (IaEditSelectedBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaEditSelectedBuild, ETriggerEvent::Started, this, &ACampPlayerCharacter::EditSelectedBuild);
	}

	if (IaInteract != nullptr)
	{
		EnhancedInputComponent->BindAction(IaInteract, ETriggerEvent::Started, this, &ACampPlayerCharacter::Interact);
	}
}

void ACampPlayerCharacter::ToggleBuildMode()
{
	if (CampBuildComponent != nullptr)
	{
		SetBuildMode(!CampBuildComponent->IsBuildModeEnabled());
	}
}

void ACampPlayerCharacter::SetBuildMode(bool bEnabled)
{
	if (CampBuildComponent == nullptr)
	{
		return;
	}

	CampBuildComponent->SetBuildMode(bEnabled);

	if (bEnabled)
	{
		AddBuildMappingContext();
		ShowBuildShortcutHintWidget();
	}
	else
	{
		HideBuildingListWidget();
		HideBuildShortcutHintWidget();
		RemoveBuildMappingContext();
	}
}

void ACampPlayerCharacter::ConfirmBuild()
{
	if (CampBuildComponent != nullptr)
	{
		CampBuildComponent->ConfirmBuild();
	}
}

void ACampPlayerCharacter::CancelBuild()
{
	if (CampBuildComponent != nullptr)
	{
		CampBuildComponent->CancelBuild();
		HideBuildingListWidget();
	}
}

void ACampPlayerCharacter::ToggleBuildList()
{
	if (CampBuildComponent == nullptr)
	{
		return;
	}

	CampBuildComponent->ToggleBuildList();
	if (CampBuildComponent->IsBuildListOpen())
	{
		ShowBuildingListWidget();
	}
	else
	{
		HideBuildingListWidget();
	}
}

void ACampPlayerCharacter::RotateBuild(const FInputActionValue& Value)
{
	if (CampBuildComponent == nullptr)
	{
		return;
	}

	const float InputValue = Value.Get<float>();
	if (FMath::IsNearlyZero(InputValue))
	{
		return;
	}

	const float Direction = InputValue > 0.0f ? 1.0f : -1.0f;
	CampBuildComponent->RotatePreview(Direction * CampBuildComponent->RotateStepDegrees);
}

void ACampPlayerCharacter::ChangeBuildType(const FInputActionValue& Value)
{
	if (CampBuildComponent == nullptr)
	{
		return;
	}

	const float InputValue = Value.Get<float>();
	if (FMath::IsNearlyZero(InputValue))
	{
		return;
	}

	CampBuildComponent->ChangeSelectedBuilding(InputValue > 0.0f ? 1 : -1);
}

void ACampPlayerCharacter::ChangeBuildTier(const FInputActionValue& Value)
{
	if (CampBuildComponent == nullptr)
	{
		return;
	}

	const float InputValue = Value.Get<float>();
	if (FMath::IsNearlyZero(InputValue))
	{
		return;
	}

	CampBuildComponent->ChangeSelectedTier(InputValue > 0.0f ? 1 : -1);
}

void ACampPlayerCharacter::ToggleBuildEditMode()
{
	if (CampBuildComponent != nullptr)
	{
		CampBuildComponent->ToggleBuildEditMode();
	}
}

void ACampPlayerCharacter::DeleteSelectedBuild()
{
	if (CampBuildComponent != nullptr)
	{
		CampBuildComponent->DeleteSelectedBuild();
	}
}

void ACampPlayerCharacter::MoveSelectedBuild()
{
	if (CampBuildComponent != nullptr)
	{
		CampBuildComponent->MoveSelectedBuild();
	}
}

void ACampPlayerCharacter::EditSelectedBuild()
{
	if (CampBuildComponent != nullptr)
	{
		CampBuildComponent->EditSelectedBuild();
	}
}

void ACampPlayerCharacter::Interact()
{
	CompleteLookedAtConstructionSite();
}

bool ACampPlayerCharacter::CompleteLookedAtConstructionSite() const
{
	if (PlayerController == nullptr || PlayerController->PlayerCameraManager == nullptr)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const FVector TraceStart = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FVector TraceEnd = TraceStart + PlayerController->PlayerCameraManager->GetCameraRotation().Vector() * InteractionTraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	if (!World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	AConstructionSiteActor* ConstructionSiteActor = Cast<AConstructionSiteActor>(HitResult.GetActor());
	if (ConstructionSiteActor == nullptr)
	{
		return false;
	}

	return ConstructionSiteActor->CompleteConstruction() != nullptr;
}

void ACampPlayerCharacter::AddBuildMappingContext()
{
	if (PlayerController == nullptr || ImcBuild == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsys =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (Subsys != nullptr)
	{
		Subsys->AddMappingContext(ImcBuild, 10);
	}
}

void ACampPlayerCharacter::RemoveBuildMappingContext()
{
	if (PlayerController == nullptr || ImcBuild == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsys =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (Subsys != nullptr)
	{
		Subsys->RemoveMappingContext(ImcBuild);
	}
}

void ACampPlayerCharacter::ShowBuildingListWidget()
{
	if (BuildingListWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Building list widget is not assigned. Set BuildingListWidgetClass on BP_CampPlayerCharacter."));
		return;
	}

	if (PlayerController == nullptr)
	{
		return;
	}

	if (BuildingListWidgetInstance == nullptr)
	{
		BuildingListWidgetInstance = CreateWidget<UBuildingListWidget>(PlayerController, BuildingListWidgetClass);
		if (BuildingListWidgetInstance != nullptr)
		{
			BuildingListWidgetInstance->InitializeBuildingList(CampBuildComponent, BuildingSelectionComponent);
			BuildingListWidgetInstance->AddToViewport();
		}
	}

	if (BuildingListWidgetInstance != nullptr)
	{
		BuildingListWidgetInstance->InitializeBuildingList(CampBuildComponent, BuildingSelectionComponent);
		BuildingListWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		PlayerController->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(BuildingListWidgetInstance->TakeWidget());
		PlayerController->SetInputMode(InputMode);
	}
}

void ACampPlayerCharacter::HideBuildingListWidget()
{
	if (BuildingListWidgetInstance != nullptr)
	{
		BuildingListWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (PlayerController != nullptr)
	{
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void ACampPlayerCharacter::ShowBuildShortcutHintWidget()
{
	if (BuildShortcutHintWidgetClass == nullptr || PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Build shortcut hint widget class is not assigned. Set BuildShortcutHintWidgetClass on BP_CampPlayerCharacter."));
		return;
	}

	if (BuildShortcutHintWidgetInstance == nullptr)
	{
		BuildShortcutHintWidgetInstance = CreateWidget<UBuildShortcutHintWidget>(PlayerController, BuildShortcutHintWidgetClass);
		if (BuildShortcutHintWidgetInstance != nullptr)
		{
			BuildShortcutHintWidgetInstance->AddToViewport(10);
		}
	}

	if (BuildShortcutHintWidgetInstance != nullptr)
	{
		RefreshBuildShortcutHintWidget();
		BuildShortcutHintWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void ACampPlayerCharacter::HideBuildShortcutHintWidget()
{
	if (BuildShortcutHintWidgetInstance != nullptr)
	{
		BuildShortcutHintWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ACampPlayerCharacter::HandleBuildStateChanged(ECampBuildState NewBuildState)
{
	RefreshBuildShortcutHintWidget();
}

void ACampPlayerCharacter::HandleBuildHoverChanged(bool bHasHoveredBuildable)
{
	RefreshBuildShortcutHintWidget();
}

void ACampPlayerCharacter::RefreshBuildShortcutHintWidget()
{
	if (BuildShortcutHintWidgetInstance == nullptr || CampBuildComponent == nullptr)
	{
		return;
	}

	BuildShortcutHintWidgetInstance->RefreshForBuildState(
		CampBuildComponent->GetBuildState(),
		CampBuildComponent->HasHoveredBuildable());
}
