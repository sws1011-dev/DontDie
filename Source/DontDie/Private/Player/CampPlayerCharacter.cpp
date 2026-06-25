// Fill out your copyright notice in the Description page of Project Settings.


#include "player/CampPlayerCharacter.h"

#include "CampBuildComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ACampPlayerCharacter::ACampPlayerCharacter()
{
	CampBuildComponent = CreateDefaultSubobject<UCampBuildComponent>(TEXT("CampBuildComponent"));
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

	if (IaChangeBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaChangeBuild, ETriggerEvent::Started, this, &ACampPlayerCharacter::ChangeBuild);
	}

	if (IaUpgradeBuild != nullptr)
	{
		EnhancedInputComponent->BindAction(IaUpgradeBuild, ETriggerEvent::Triggered, this, &ACampPlayerCharacter::UpgradeBuild);
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

void ACampPlayerCharacter::ChangeBuild(const FInputActionValue& Value)
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

void ACampPlayerCharacter::UpgradeBuild(const FInputActionValue& Value)
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
