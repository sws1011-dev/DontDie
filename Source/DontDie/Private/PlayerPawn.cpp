// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"

#include "DontDieGameModeBase.h"
#include "GameOverWidget.h"
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
	CapsuleComp->InitCapsuleSize(55.f, 140.f);
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
	CurrentLife = LifeCount;

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

	// 영구 업그레이드 수치 적용
	ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->ApplyPersistentUpgrades(this);
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

		// 무기에 자신의 투사체 개수(ProjectileCount)를 전달하여 점사 발사를 수행합니다.
		CurrentWeapon->Fire(ProjectileCount, DamageMultiplier);
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

void APlayerPawn::OnReceiveDamage(float DamageAmount)
{
	// 1. 데미지 적용 (HP가 0 이하로 떨어지지 않게 Clamp)
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

	// HUD가 있다면 HP 바 갱신 (예시: RefreshHUD 호출 혹은 내부 UI 연동)
	RefreshHUD();

	// 2. HP가 0이 되었는지 체크
	if (CurrentHP <= 0.0f)
	{
		// 목숨 차감 함수 호출
		DecreaseLife();
	}
}

void APlayerPawn::DecreaseLife()
{
	CurrentLife = FMath::Max(0, CurrentLife - 1);

	if (HUDWidget != nullptr)
	{
		HUDWidget->UpdateLifeText(CurrentLife);
	}

	if (CurrentLife > 0)
	{
		// 1. 목숨이 남아있으므로 체력을 다시 최대치로 리셋(부활)
		CurrentHP = MaxHP;

		// 2. 변경된 체력과 목숨 정보를 HUD에 즉시 반영
		RefreshHUD();

		// 필요하다면 여기에 리스폰 무적 시간 처리나 이펙트 코드를 추가할 수 있습니다.
		UE_LOG(LogTemp, Warning, TEXT("플레이어 사망! 체력 리셋 완료. 남은 목숨: %d"), CurrentLife);
	}
	else
	{
		ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		int32 EarnedGold = 0;
		int32 TotalGold = 0;
		if (GM)
		{
			EarnedGold = GM->CurrentGold;
			GM->FinalizeGold();
			TotalGold = GM->TotalGold;
		}

		if (GameOverWidgetClass != nullptr)
		{
			UGameOverWidget* GameOverUI = CreateWidget<UGameOverWidget>(GetWorld(), GameOverWidgetClass);

			if (GameOverUI != nullptr)
			{
				GameOverUI->SetupResults(EarnedGold, TotalGold);
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
	ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());

	if (HUDWidget != nullptr)
	{
		if (MaxHP > 0.0f)
		{
			float HPPercent = CurrentHP / MaxHP;
			HUDWidget->UpdateHPBar(HPPercent);
		}

		if (GM != nullptr)
		{
			float Progress = GM->GetWaveProgress();
			HUDWidget->UpdateWaveProgress(Progress);

			HUDWidget->UpdateWaveStageText(GM->CurrentWave);
		}

		HUDWidget->UpdateLifeText(CurrentLife);

		if (CurrentWeapon != nullptr)
		{
			HUDWidget->UpdateAmmoText(CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
		}

		if (GM != nullptr)
		{
			HUDWidget->UpdateGoldText(GM->CurrentGold);
		}
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
