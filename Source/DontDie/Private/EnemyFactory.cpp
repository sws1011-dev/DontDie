// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFactory.h"

#include "EnemyActor.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
// Sets default values
AEnemyFactory::AEnemyFactory()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SetRootComponent(SpawnArea);

	SpawnArea->SetBoxExtent(FVector(500.f, 500.f, 10.f));
}

// Called when the game starts or when spawned
void AEnemyFactory::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AEnemyFactory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentTime += DeltaTime;

	if (CurrentTime > DelayTime)
	{
		CurrentTime = 0;
		int32 SpawnCount = FMath::RandRange(1, 5);
		for (int32 i = 0; i < SpawnCount; i++)
		{
			FVector RandomLocation = UKismetMathLibrary::RandomPointInBoundingBox(
				SpawnArea->GetComponentLocation(),
				SpawnArea->GetScaledBoxExtent()
			);
			RandomLocation.Z = GetActorLocation().Z;

			AEnemyActor* NewEnemy = GetWorld()->SpawnActor<AEnemyActor>(Enemy, RandomLocation, GetActorRotation());

			if (NewEnemy)
			{
				NewEnemy->MoveSpeed = FMath::RandRange(300.f, 500.f);
			}
		}
	}
}
