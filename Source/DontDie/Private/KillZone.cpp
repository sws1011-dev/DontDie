#include "KillZone.h"

#include "DontDieGameModeBase.h"
#include "EnemyActor.h"
#include "PlayerPawn.h"
#include "SurvivorActor.h"
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

	ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());

	// 1. 적이 킬존에 닿았을 때
	if (AEnemyActor* Enemy = Cast<AEnemyActor>(OtherActor))
	{
		if (GM)
		{
			// 적 숫자를 줄여서 웨이브 종료가 안 되는 현상을 방지
			GM->OnEnemyKilled();
		}
		OtherActor->Destroy();
	}
	// 2. 플레이어가 킬존에 닿았을 때
	else if (APlayerPawn* Player = Cast<APlayerPawn>(OtherActor))
	{
		Player->DecreaseLife();
	}
	else if (ASurvivorActor* Survivor = Cast<ASurvivorActor>(OtherActor))
	{
		if (GM)
		{
			// 게임모드에 생존자가 들고 있는 보상 점수(150점)를 더해줍니다.
			GM->AddGold(Survivor->ScoreReward);
			
			GM->OnSurvivorRemoved();
		}

		UE_LOG(LogTemp, Log, TEXT("생존자 구출 성공! 150점 획득."));
		OtherActor->Destroy();
	}
}


// Called every frame
void AKillZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
