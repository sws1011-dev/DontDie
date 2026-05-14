// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyActor.h"

#include "PlayerPawn.h"
#include "Components/BoxComponent.h"
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
		OtherActor->Destroy();
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
