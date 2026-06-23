// Fill out your copyright notice in the Description page of Project Settings.


#include "player/PlayerCharacter.h"

#include "gamemode/DontDieGameModeBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "widget/GameOverWidget.h"
#include "widget/PlayerHudWidget.h"
#include "weapon/Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(55.f, 140.f);
	// GetCapsuleComponent()->SetCollisionProfileName(TEXT("Player"));
	USkeletalMeshComponent* mesh = GetMesh();
	mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 250.f;
	SpringArmComp->SocketOffset = FVector(0.f, 50.f, 70.f);

	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = true;
	
	SpringArmComp->bDoCollisionTest = false;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	CurrentLifeCount = MaxLifeCount;

	PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* subsys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (subsys != nullptr)
		{
			subsys->AddMappingContext(ImcPlayerInput, 0);
		}

		// 화면에서 마우스 커서를 숨기고 게임 화면에 마우스를 잠금(포커싱) 처리
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(false);
		PlayerController->SetInputMode(InputModeData);
	}

	if (PlayerHudWidgetClass != nullptr)
	{
		HUDWidget = CreateWidget<UPlayerHudWidget>(GetWorld(), PlayerHudWidgetClass);
		if (HUDWidget != nullptr)
		{
			HUDWidget->AddToViewport();
			HUDWidget->UpdateLifeText(CurrentLifeCount);
			UpdateReloadingHUD(false);
			UE_LOG(LogTemp, Warning, TEXT("HUD Created and Added to Viewport!"));
		}
	}

	// 무기 스폰 및 부착 (기존 코드 유지)
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
			CurrentWeapon->AttachToComponent(GetMesh(), AttachRules, TEXT("WeaponSocket"));

			UpdateAmmoHUD(CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IaMove, ETriggerEvent::Triggered, this, &APlayerCharacter::OnInputMovement);
		EnhancedInputComponent->BindAction(IaLook, ETriggerEvent::Triggered, this, &APlayerCharacter::OnInputLook);
		EnhancedInputComponent->BindAction(IaFire, ETriggerEvent::Started, this, &APlayerCharacter::Fire);
		EnhancedInputComponent->BindAction(IaReload, ETriggerEvent::Started, this, &APlayerCharacter::Reload);
	}
}

void APlayerCharacter::OnInputLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 마우스 X 입력은 좌우 회전(Yaw), Y 입력은 상하 시선 회전(Pitch)으로 컨트롤러에 더해줍니다.
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void APlayerCharacter::OnInputMovement(const struct FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	UE_LOG(LogTemp, Warning, TEXT("Input Vector: X = %f, Y = %f"), MovementVector.X, MovementVector.Y);

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 사용자의 IMC 세팅(Y: W/S, X: A/D)에 맞춰 정확히 주입
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Fire()
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

void APlayerCharacter::Reload()
{
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->Reload();
	}
}

float APlayerCharacter::GetCalculatedDamage()
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

void APlayerCharacter::OnReceiveDamage(float DamageAmount)
{
	// 1. 데미지 적용 (HP가 0 이하로 떨어지지 않게 Clamp)
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

	// HUD가 있다면 HP 바 갱신 (예시: RefreshHUD 호출 혹은 내부 UI 연동)
	RefreshHUD();

	// 2. HP가 0이 되었는지 체크
	if (CurrentHP <= 0.0f)
	{
		// 목숨 차감 함수 호출
		DecreaseLifeCount();
	}
}

void APlayerCharacter::DecreaseLifeCount()
{
	CurrentLifeCount = FMath::Max(0, CurrentLifeCount - 1);

	if (HUDWidget != nullptr)
	{
		HUDWidget->UpdateLifeText(CurrentLifeCount);
	}

	if (CurrentLifeCount > 0)
	{
		// 1. 목숨이 남아있으므로 체력을 다시 최대치로 리셋(부활)
		CurrentHP = MaxHP;

		// 2. 변경된 체력과 목숨 정보를 HUD에 즉시 반영
		RefreshHUD();

		// 필요하다면 여기에 리스폰 무적 시간 처리나 이펙트 코드를 추가할 수 있습니다.
		UE_LOG(LogTemp, Warning, TEXT("플레이어 사망! 체력 리셋 완료. 남은 목숨: %d"), CurrentLifeCount);
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

void APlayerCharacter::RefreshHUD()
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

		HUDWidget->UpdateLifeText(CurrentLifeCount);

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

void APlayerCharacter::UpdateAmmoHUD(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (HUDWidget != nullptr)
	{
		HUDWidget->UpdateAmmoText(CurrentAmmo, MaxAmmo);
	}
}

void APlayerCharacter::UpdateReloadingHUD(bool bIsReloading)
{
	if (HUDWidget != nullptr)
	{
		HUDWidget->SetReloadingVisibility(bIsReloading);
	}
}
