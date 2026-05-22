// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivorActor.h"

#include "DontDieGameModeBase.h"
#include "EnemyDamagedWidget.h"
#include "EnemyHpWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ASurvivorActor::ASurvivorActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(55.f, 140.f);
	SetRootComponent(CapsuleComp);

	// 총알과 킬존에 닿아야 하므로 콜리전 설정
	CapsuleComp->SetCollisionProfileName(TEXT("Enemy"));

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);

	HpWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpWidgetComponent"));
	HpWidgetComp->SetupAttachment(CapsuleComp);
	HpWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	HpWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void ASurvivorActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	MoveDirection = GetActorForwardVector();
}

void ASurvivorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewLocation = GetActorLocation() + MoveDirection * MoveSpeed * DeltaTime;
	SetActorLocation(NewLocation, true);
}

void ASurvivorActor::ShowDamage(float DamageAmount, FVector HitLocation)
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

void ASurvivorActor::TakeDamage(float DamageAmount, FVector HitLocation)
{
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);
	ShowDamage(DamageAmount, HitLocation);
	UpdateHpUI();

	if (CurrentHP <= 0)
	{
		if (ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			GM->OnSurvivorRemoved();
		}
		Destroy();
	}
}

void ASurvivorActor::UpdateHpUI()
{
	if (HpWidgetComp == nullptr) return;

	// 1. 💡 [핵심] GetWidget() 대신 GetUserWidgetObject()를 사용하여 안전하게 인스턴스를 검사합니다.
	UUserWidget* TmpWidget = HpWidgetComp->GetUserWidgetObject();

	// 2. 만약 위젯이 아직 생성되지 않았다면, 강제로 인스턴스를 즉시 빌드해오도록 만듭니다.
	if (TmpWidget == nullptr)
	{
		HpWidgetComp->InitWidget();
		TmpWidget = HpWidgetComp->GetUserWidgetObject(); // 생성된 위젯을 다시 할당
	}

	// 3. 캐스팅 검사 수행
	UEnemyHpWidget* HpBarUI = Cast<UEnemyHpWidget>(TmpWidget);

	if (HpBarUI != nullptr && MaxHP > 0)
	{
		// 정상적으로 위젯을 찾았다면 퍼센트를 계산해 UI 바를 갱신합니다.
		float HPPercent = (float)CurrentHP / (float)MaxHP;
		HpBarUI->UpdateHealthBar(HPPercent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: HP 위젯 캐스팅 실패 또는 위젯 클래스가 등록되지 않음!"), *GetName());
	}
}
