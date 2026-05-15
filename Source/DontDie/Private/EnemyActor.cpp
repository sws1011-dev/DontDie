// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyActor.h"

#include "DontDieGameModeBase.h"
#include "EnemyDamagedWidget.h"
#include "EnemyHpWidget.h"
#include "PlayerPawn.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AEnemyActor::AEnemyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	SetRootComponent(BoxComp);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	StaticMeshComp->SetupAttachment(BoxComp);

	FVector boxSize = FVector(50.0f, 50.0f, 50.0f);
	BoxComp->SetBoxExtent(boxSize);

	BoxComp->SetCollisionProfileName(TEXT("Enemy"));

	HpWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpWidgetComponent"));

	HpWidgetComp->SetupAttachment(BoxComp);

	HpWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

	HpWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

// Called when the game starts or when spawned
void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;

	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc != nullptr)
	{
		TargetPlayer = Cast<APlayerPawn>(pc->GetPawn());
	}
	dir = GetActorForwardVector();

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyActor::OnEnemyOverlap);
}

void AEnemyActor::OnEnemyOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                 const FHitResult& SweepResult)
{
	// 충돌한 상대 액터를 APlayerActor 클래스로 변환
	APlayerPawn* player = Cast<APlayerPawn>(OtherActor);
	if (player != nullptr)
	{
		// 충돌된 플레이어 제거
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
		ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM) GM->OnEnemyKilled();
       
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			APlayerPawn* Player = Cast<APlayerPawn>(PC->GetPawn());
			if (Player)
			{
				Player->RefreshHUD();
			}
		}
       
		// 반드시 조건문 안에서 Destroy해야 죽었을 때만 사라집니다!
		Destroy(); 
	}
	// Destroy(); <-- 이 녀석이 밖에 있어서 한 대만 맞아도 삭제되었던 겁니다!
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
