#include "KillZone.h"

#include "DontDieGameModeBase.h"
#include "EnemyActor.h"
#include "PlayerPawn.h"
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
	if (OtherActor == nullptr) return;

	// 1. 적이 킬존에 닿았을 때
	if (AEnemyActor* Enemy = Cast<AEnemyActor>(OtherActor))
	{
		ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			// 적 숫자를 줄여서 웨이브 종료가 안 되는 현상을 방지
			GM->OnEnemyKilled();
		}
	}
	// 2. 플레이어가 킬존에 닿았을 때
	else if (APlayerPawn* Player = Cast<APlayerPawn>(OtherActor))
	{
		Player->DecreaseLife();
	}

	OtherActor->Destroy();
}


// Called every frame
void AKillZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
