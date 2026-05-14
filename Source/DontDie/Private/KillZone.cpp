// Fill out your copyright notice in the Description page of Project Settings.


#include "KillZone.h"

#include "Components/BoxComponent.h"


// Sets default values
AKillZone::AKillZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	BoxComp = CreateDefaultSubobject<UBoxComponent>("Kill Zone Box");
	BoxComp->SetMobility(EComponentMobility::Static);
	FVector boxSize = FVector(100.0f, 2000.f, 100.0f);

	BoxComp->SetBoxExtent(boxSize);
	BoxComp->SetCollisionProfileName(TEXT("KillZone"));
}

// Called when the game starts or when spawned
void AKillZone::BeginPlay()
{
	Super::BeginPlay();

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AKillZone::OnKillZoneOverlap);
}

void AKillZone::OnKillZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	OtherActor->Destroy();
}


// Called every frame
void AKillZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
