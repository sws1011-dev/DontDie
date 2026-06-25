// Fill out your copyright notice in the Description page of Project Settings.


#include "player/BasePlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

ABasePlayerCharacter::ABasePlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(55.0f, 140.0f);

	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 250.0f;
	SpringArmComp->SocketOffset = FVector(0.0f, 50.0f, 70.0f);
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->bDoCollisionTest = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = true;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		PlayerController = GetWorld() != nullptr ? GetWorld()->GetFirstPlayerController() : nullptr;
	}

	if (PlayerController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (Subsys != nullptr && ImcDefault != nullptr)
		{
			Subsys->AddMappingContext(ImcDefault, 0);
		}

		PlayerController->bShowMouseCursor = false;
		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(false);
		PlayerController->SetInputMode(InputModeData);
	}
}

void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		return;
	}

	if (IaMove != nullptr)
	{
		EnhancedInputComponent->BindAction(IaMove, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::OnInputMovement);
	}

	if (IaLook != nullptr)
	{
		EnhancedInputComponent->BindAction(IaLook, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::OnInputLook);
	}
}

void ABasePlayerCharacter::OnInputMovement(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller == nullptr)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABasePlayerCharacter::OnInputLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller == nullptr)
	{
		return;
	}

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}
