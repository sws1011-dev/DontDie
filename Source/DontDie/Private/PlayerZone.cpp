// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerZone.h"

#include "Components/BoxComponent.h"


// Sets default values
APlayerZone::APlayerZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Player Zone Box"));
	BoxComp->SetMobility(EComponentMobility::Static);
	FVector boxSize = FVector(100.0f, 2000.f, 100.0f);
	BoxComp->SetBoxExtent(boxSize);
	BoxComp->SetCollisionProfileName(TEXT("PlayerZone"));
}

// Called when the game starts or when spawned
void APlayerZone::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlayerZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
