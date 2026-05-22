// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyActor.h"

#include "DontDieGameModeBase.h"
#include "EnemyDamagedWidget.h"
#include "EnemyHpWidget.h"
#include "PlayerPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AEnemyActor::AEnemyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComp->InitCapsuleSize(55.f, 140.f);
	SetRootComponent(CapsuleComp);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(CapsuleComp);

	CapsuleComp->SetCollisionProfileName(TEXT("Enemy"));

	HpWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpWidgetComponent"));
	HpWidgetComp->SetupAttachment(CapsuleComp);
	HpWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	HpWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

// Called when the game starts or when spawned
void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	MyGameMode = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());

	if (MyGameMode)
	{
		// 웨이브가 5씩 진행될 때마다 스탯 증가 (1~5: 기본, 6~10: +1단계, 11~15: +2단계...)
		int32 ScalingStep = (MyGameMode->CurrentWave - 1) / 5;

		if (ScalingStep > 0)
		{
			MaxHP += (ScalingStep * MyGameMode->HealthIncrementPer5Waves);
			AttackPower += (ScalingStep * MyGameMode->AttackPowerIncrementPer5Waves);

			UE_LOG(LogTemp, Log, TEXT("Enemy Scaled! Wave: %d, HP: %f, Attack: %f"),
			       MyGameMode->CurrentWave, MaxHP, AttackPower);
		}
	}

	CurrentHP = MaxHP;

	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc != nullptr)
	{
		TargetPlayer = Cast<APlayerPawn>(pc->GetPawn());
	}
	dir = GetActorForwardVector();

	CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyActor::OnEnemyOverlap);
}

void AEnemyActor::OnEnemyOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                 const FHitResult& SweepResult)
{
}

// Called every frame
void AEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 타겟 플레이어가 있고, 현재 공격 중이 아닐 때만 로직 수행
	if (TargetPlayer != nullptr && !bIsAttacking)
	{
		// 플레이어와의 실제 거리 계산
		float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

		// 2. 사거리 안에 들어왔는지 검사
		if (DistanceToPlayer <= AttackRange)
		{
			// 이동 방향 벡터를 0으로 만들어 멈추게 하고 공격 실행
			dir = FVector::ZeroVector;
			Attack();
		}
		else
		{
			// 사거리 밖이라면 기존 추적 및 회전 로직 실행
			dir = TargetPlayer->GetActorLocation() - GetActorLocation();
			dir.Normalize();

			FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(
				GetActorLocation(), TargetPlayer->GetActorLocation());
			SetActorRotation(FRotator(0, TargetRot.Yaw, 0));
		}
	}
	FVector newLocation = GetActorLocation() + dir * MoveSpeed * DeltaTime;
	SetActorLocation(newLocation, true);
}

void AEnemyActor::ShowDamage(float DamageAmount, FVector HitLocation)
{
	if (DamagedWidget != nullptr)
	{
		UEnemyDamagedWidget* damagedWidget = CreateWidget<UEnemyDamagedWidget>(GetWorld(), DamagedWidget);

		if (damagedWidget != nullptr)
		{
			damagedWidget->AddToViewport();
			damagedWidget->SetDamageText(DamageAmount);

			FVector2D ScreenPosition;
			if (UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), HitLocation,
			                                           ScreenPosition))
			{
				damagedWidget->SetPositionInViewport(ScreenPosition);
			}
		}
	}
}

void AEnemyActor::TakeDamage(float DamageAmount, FVector HitLocation)
{
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);
	ShowDamage(DamageAmount, HitLocation);
	UpdateHpUI();

	if (CurrentHP <= 0)
	{
		if (MyGameMode)
		{
			MyGameMode->OnEnemyKilled();

			// 플레이어의 재화 획득 배율 반영
			float Multiplier = 1.0f;
			if (TargetPlayer)
			{
				Multiplier = TargetPlayer->CurrencyMultiplier;
			}

			int32 FinalGold = FMath::RoundToInt(CurrencyReward * Multiplier);
			MyGameMode->AddGold(FinalGold);
		}
		Destroy();
	}
}

void AEnemyActor::UpdateHpUI()
{
	UUserWidget* TmpWidget = HpWidgetComp->GetWidget();
	UEnemyHpWidget* HpBarUI = Cast<UEnemyHpWidget>(TmpWidget);

	if (HpBarUI != nullptr && MaxHP > 0)
	{
		float HPPercent = (float)CurrentHP / (float)MaxHP;
		HpBarUI->UpdateHealthBar(HPPercent);
	}
}

void AEnemyActor::Attack()
{
	if (bIsAttacking || !AttackMontage || !SkeletalMeshComponent) return;

	bIsAttacking = true;

	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (AnimInstance)
	{
		// 몽타주 재생
		AnimInstance->Montage_Play(AttackMontage, 2.5f);

		// 몽타주가 끝났을 때 호출될 함수를 링크(바인딩) 해줍니다.
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &AEnemyActor::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AttackMontage);
	}
}

void AEnemyActor::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}

void AEnemyActor::DoHitCheck()
{
	if (TargetPlayer != nullptr)
	{
		// 때리는 순간 플레이어와의 거리 계산
		float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

		// 사거리 내에 있다면 즉시 데미지!
		if (DistanceToPlayer <= 250)
		{
			TargetPlayer->OnReceiveDamage(AttackPower);
			UE_LOG(LogTemp, Log, TEXT("좀비 타격 순간 적중! 데미지: %f"), AttackPower);
		}
	}
}
