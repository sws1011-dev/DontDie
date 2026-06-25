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

void ACampPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACampPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		EnhancedInputComponent->BindAction(IaRotateBuild, ETriggerEvent::Triggered, this, &ACampPlayerCharacter::RotateBuild);
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

	const float WheelValue = Value.Get<float>();
	if (FMath::IsNearlyZero(WheelValue))
	{
		return;
	}

	const float Direction = WheelValue > 0.0f ? 1.0f : -1.0f;
	CampBuildComponent->RotatePreview(Direction * CampBuildComponent->RotateStepDegrees);
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
