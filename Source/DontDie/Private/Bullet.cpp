// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"

#include "EnemyActor.h"
#include "PlayerPawn.h"
#include "SurvivorActor.h"
#include "Components/BoxComponent.h"


// Sets default values
ABullet::ABullet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("My Box Collider"));
	SetRootComponent(BoxComp);

	FVector boxSize = FVector(20.0f, 5.0f, 5.0f);
	BoxComp->SetBoxExtent(boxSize);

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
	float FinalDamage = 0.0f;
	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc != nullptr)
	{
		APlayerPawn* player = Cast<APlayerPawn>(pc->GetPawn());
		if (player != nullptr)
		{
			FinalDamage = player->GetCalculatedDamage();
		}
	}

	AEnemyActor* enemy = Cast<AEnemyActor>(OtherActor);
	if (enemy != nullptr)
	{
		enemy->TakeDamage(FinalDamage, GetActorLocation());
	}
	else
	{
		ASurvivorActor* survivor = Cast<ASurvivorActor>(OtherActor);
		if (survivor != nullptr)
		{
			survivor->TakeDamage(FinalDamage, GetActorLocation());
		}
	}
	Destroy();
}
