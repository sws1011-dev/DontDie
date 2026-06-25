// Fill out your copyright notice in the Description page of Project Settings.


#include "player/CampPlayerCharacter.h"

#include "BuildingSelectionComponent.h"
#include "CampBuildComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ACampPlayerCharacter::ACampPlayerCharacter()
{
	BuildingSelectionComponent = CreateDefaultSubobject<UBuildingSelectionComponent>(TEXT("BuildingSelectionComponent"));
	CampBuildComponent = CreateDefaultSubobject<UCampBuildComponent>(TEXT("CampBuildComponent"));
	CampBuildComponent->SetBuildingSelectionComponent(BuildingSelectionComponent);
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
		EnhancedInputComponent->BindAction(IaChangeBuildTier, ETriggerEvent::Triggered, this, &ACampPlayerCharacter::ChangeBuildTier);
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
	}
	else
	{
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
	SetBuildMode(false);
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
