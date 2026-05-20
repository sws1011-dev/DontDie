// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyActor.h"

#include "DontDieGameModeBase.h"
#include "EnemyDamagedWidget.h"
#include "EnemyHpWidget.h"
#include "PlayerPawn.h"
#include "Components/BoxComponent.h"
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
	CapsuleComp->InitCapsuleSize(55.f, 95.f);
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
	// 충돌한 상대 액터를 APlayerActor 클래스로 변환
	APlayerPawn* player = Cast<APlayerPawn>(OtherActor);
	if (player != nullptr)
	{
		// 게임모드에 자폭 소식을 알려 생존 적 숫자를 줄입니다.
		if (MyGameMode)
		{
			MyGameMode->OnEnemyOverlapDestroyed();
		}

		// 충돌된 플레이어 생명 감소
		player->DecreaseLife();
		// 적 자신도 제거
		Destroy();
	}
}

// Called every frame
void AEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetPlayer != nullptr)
	{
		dir = TargetPlayer->GetActorLocation() - GetActorLocation();
		dir.Normalize();
		FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),
		                                                            TargetPlayer->GetActorLocation());
		SetActorRotation(FRotator(0, TargetRot.Yaw, 0));
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

	// HpBarUI가 null인지 체크하는 로직 추가가 안전합니다.
	if (HpBarUI != nullptr && MaxHP > 0)
	{
		// 실수 연산을 위해 float 보장
		float HPPercent = (float)CurrentHP / (float)MaxHP;
		HpBarUI->UpdateHealthBar(HPPercent);
	}
}
