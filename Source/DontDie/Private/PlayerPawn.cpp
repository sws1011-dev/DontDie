// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"

#include "DontDieGameModeBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerHudWidget.h"
#include "Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
APlayerPawn::APlayerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	CapsuleComp->InitCapsuleSize(55.f, 95.f);
	SetRootComponent(CapsuleComp);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("My Skeletal Mesh Component"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	CapsuleComp->SetCollisionProfileName(TEXT("Player"));
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;

	PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* subsys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (subsys != nullptr)
		{
			subsys->AddMappingContext(ImcPlayerInput, 0);
		}
	}
	
	if (HUDClass != nullptr)
	{
		HUDWidget = CreateWidget<UPlayerHudWidget>(GetWorld(), HUDClass);
		if (HUDWidget != nullptr)
		{
			HUDWidget->AddToViewport();
			HUDWidget->UpdateLifeText(CurrentLife);
			UpdateReloadingHUD(false);
			UE_LOG(LogTemp, Warning, TEXT("HUD Created and Added to Viewport!"));
		}
	}

	if (DefaultWeaponClass != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, GetActorLocation(), GetActorRotation(),
		                                                SpawnParams);

		if (CurrentWeapon != nullptr)
		{
			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
			CurrentWeapon->AttachToComponent(SkeletalMeshComp, AttachRules, TEXT("WeaponSocket"));
			
			UpdateAmmoHUD(CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
		}
	}
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector dir = FVector(MoveInput.Y, MoveInput.X, 0);
	dir.Normalize();

	FVector vector = dir * MoveSpeed * DeltaTime;

	SetActorLocation(GetActorLocation() + FVector(vector.X, 0, 0), true);
	SetActorLocation(GetActorLocation() + FVector(0, vector.Y, 0), true);

	if (PlayerController != nullptr)
	{
		FVector WorldLocation, WorldDirection;

		if (PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			float Distance = (GetActorLocation().Z - WorldLocation.Z) / WorldDirection.Z;
			FVector IntersectLocation = WorldLocation + (WorldDirection * Distance);

			FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), IntersectLocation);
			SetActorRotation(TargetRotation);
		}
	}
}

void APlayerPawn::OnInputMove(const struct FInputActionValue& value)
{
	MoveInput = value.Get<FVector2D>();
}

void APlayerPawn::Fire()
{
	if (CurrentWeapon != nullptr)
	{
		float DamageMultiplier = 1.0f;
		if (FMath::FRand() < CritChance)
		{
			DamageMultiplier = 2.0f;
		}

		CurrentWeapon->Fire(DamageMultiplier);
	}
}

void APlayerPawn::Reload()
{
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->Reload();
	}
}

float APlayerPawn::GetCalculatedDamage()
{
	if (CurrentWeapon != nullptr)
	{
		float FinalDamage = CurrentWeapon->BaseDamage;
		if (FMath::FRand() < CritChance)
		{
			FinalDamage *= 2.0f;
		}
		return FinalDamage;
	}
	return 0;
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* eic = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (eic != nullptr)
	{
		eic->BindAction(IaMove, ETriggerEvent::Triggered, this, &APlayerPawn::OnInputMove);
		eic->BindAction(IaMove, ETriggerEvent::Completed, this, &APlayerPawn::OnInputMove);
		eic->BindAction(IaFire, ETriggerEvent::Started, this, &APlayerPawn::Fire);
		eic->BindAction(IaReload, ETriggerEvent::Started, this, &APlayerPawn::Reload);
	}
}

void APlayerPawn::DecreaseLife()
{
	CurrentLife = FMath::Max(0, CurrentLife - 1);

	if (HUDWidget != nullptr)
	{
		HUDWidget->UpdateLifeText(CurrentLife);
	}

	if (CurrentLife <= 0)
	{
		if (GameOverWidgetClass != nullptr)
		{
			UUserWidget* GameOverUI = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);

			if (GameOverUI != nullptr)
			{
				GameOverUI->AddToViewport();
				APlayerController* pc = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
				if (pc != nullptr)
				{
					pc->SetShowMouseCursor(true);
					pc->SetInputMode(FInputModeUIOnly());
				}
				UGameplayStatics::SetGamePaused(GetWorld(), true);
			}
		}
	}
}

void APlayerPawn::RefreshHUD()
{
	UE_LOG(LogTemp, Warning, TEXT("RefreshHUD Called!"));
	ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM && HUDWidget)
	{
		float Progress = GM->GetWaveProgress();
		HUDWidget->UpdateWaveProgress(Progress);
	}
}

void APlayerPawn::UpdateAmmoHUD(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (HUDWidget != nullptr)
	{
		HUDWidget->UpdateAmmoText(CurrentAmmo, MaxAmmo);
	}
}

void APlayerPawn::UpdateReloadingHUD(bool bIsReloading)
{
	if (HUDWidget != nullptr)
	{
		HUDWidget->SetReloadingVisibility(bIsReloading);
	}
}