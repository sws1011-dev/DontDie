// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"

#include "EnemyActor.h"
#include "PlayerPawn.h"
#include "Components/BoxComponent.h"


// Sets default values
ABullet::ABullet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("My Box Collider"));
	SetRootComponent(BoxComp);

	FVector boxSize = FVector(50.0f, 50.0f, 50.0f);
	BoxComp->SetBoxExtent(boxSize);

	BoxComp->SetWorldScale3D(FVector(0.25f, .25f, .25f));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("My Mesh Component"));
	MeshComp->SetupAttachment(BoxComp);

	BoxComp->SetCollisionProfileName(TEXT("Bullet"));
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnBulletOverlap);
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector newLocation = GetActorLocation() + GetActorForwardVector() * MoveSpeed * DeltaTime;
	SetActorLocation(newLocation);
}

void ABullet::OnBulletOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                              const FHitResult& SweepResult)
{
	// 충돌한 상대 액터를 AEnemyActor 클래스로 변환
	AEnemyActor* enemy = Cast<AEnemyActor>(OtherActor);
	if (enemy != nullptr)
	{
		// 플레이어 정보를 가져와서 공격력 확인
		APlayerController* pc = GetWorld()->GetFirstPlayerController();
		if (pc != nullptr)
		{
			APlayerPawn* player = Cast<APlayerPawn>(pc->GetPawn());
			if (player != nullptr)
			{
				// 계산된 데미지(치명타 포함) 적용
				enemy->CurrentHP -= player->GetCalculatedDamage();

				// HP가 0 이하면 제거
				if (enemy->CurrentHP <= 0)
				{
					enemy->Destroy();
				}
			}
		}
	}
	// 총알 자신도 제거
	Destroy();
}
