// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Bullet.h"
#include "PlayerPawn.h"
#include "Components/ArrowComponent.h"


// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComponent"));
	SetRootComponent(WeaponMeshComp);

	FirePosition = CreateDefaultSubobject<UArrowComponent>(TEXT("Fire Component"));
	FirePosition->SetupAttachment(WeaponMeshComp);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = MaxAmmo;
}

void AWeapon::Fire(int32 ProjectileCount, float DamageMultiplier)
{
	// 1. 재장전 중이거나 이미 점사 중이면 발사 불가
	if (bIsReloading || RemainingBurstCount > 0)
	{
		return;
	}

	// 2. 공격 속도(연사 제한) 확인 - 첫 발사 시점 기준
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < (1.0f / FireRate))
	{
		return;
	}

	// 3. 탄약 확인 (최소 한 발은 있어야 시작)
	if (CurrentAmmo <= 0)
	{
		Reload();
		return;
	}

	// 4. 점사 시작
	LastFireTime = CurrentTime;
	StartBurst(ProjectileCount, DamageMultiplier);
}

void AWeapon::StartBurst(int32 Count, float Multiplier)
{
	RemainingBurstCount = Count;
	CurrentBurstMultiplier = Multiplier;

	// 즉시 첫 발사 실행
	ExecuteShot();

	// 남은 발사수가 있다면 타이머 설정
	if (RemainingBurstCount > 0)
	{
		GetWorldTimerManager().SetTimer(BurstTimerHandle, this, &AWeapon::ExecuteShot, BurstInterval, true);
	}
}

void AWeapon::ExecuteShot()
{
	// 탄약이 없거나 모든 점사를 마쳤으면 중단
	if (CurrentAmmo <= 0 || RemainingBurstCount <= 0)
	{
		RemainingBurstCount = 0;
		GetWorldTimerManager().ClearTimer(BurstTimerHandle);
		
		if (CurrentAmmo <= 0) Reload();
		return;
	}

	// 총알 생성 로직
	if (BulletFactory)
	{
		FVector SpawnLocation = FirePosition->GetComponentLocation();
		FRotator SpawnRotation = FRotator::ZeroRotator;
		
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			SpawnRotation = OwnerPawn->GetActorRotation();
		}
		else
		{
			SpawnRotation = FirePosition->GetComponentRotation();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(BulletFactory, SpawnLocation, SpawnRotation, SpawnParams);

		if (Bullet != nullptr)
		{
			// 데미지나 추가 정보 설정 가능 (예: Bullet->Damage *= CurrentBurstMultiplier)
			
			CurrentAmmo--;
			RemainingBurstCount--;
			
			UpdatePlayerHUD();
		}
	}

	// 모든 발사를 마쳤으면 타이머 해제
	if (RemainingBurstCount <= 0)
	{
		GetWorldTimerManager().ClearTimer(BurstTimerHandle);
	}
}

void AWeapon::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo) return;

	bIsReloading = true;
	UE_LOG(LogTemp, Warning, TEXT("Reload Started... Wait %f sec"), ReloadSpeed);


	APlayerPawn* Player = Cast<APlayerPawn>(GetOwner());
	if (Player)
	{
		Player->UpdateReloadingHUD(true);
	}
	
	// 재장전 타이머 돌리기
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AWeapon::OnReloadComplete,
		ReloadSpeed,
		false
	);
}

void AWeapon::OnReloadComplete()
{
	bIsReloading = false;
	CurrentAmmo = MaxAmmo;
	UE_LOG(LogTemp, Warning, TEXT("Reload Complete! Ammo: %d"), CurrentAmmo);

	APlayerPawn* Player = Cast<APlayerPawn>(GetOwner());
	if (Player)
	{
		Player->UpdateReloadingHUD(false);
	}
	
	UpdatePlayerHUD();
}

void AWeapon::UpdatePlayerHUD()
{
	// 이 무기의 소유자(Owner)를 플레이어 Pawn으로 캐스팅합니다.
	APlayerPawn* Player = Cast<APlayerPawn>(GetOwner());
	if (Player != nullptr)
	{
		// 플레이어에게 현재 남은 탄약 정보를 넘겨주며 UI 갱신을 요청합니다.
		Player->UpdateAmmoHUD(CurrentAmmo, MaxAmmo);
	}
}
