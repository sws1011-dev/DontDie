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

void AWeapon::Fire(float DamageMultiplier)
{
	// 1. 재장전 중인지 확인
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon: Cannot Fire! Now Reloading..."));
		return;
	}

	// 2. 공격 속도(연사 제한) 확인
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < (1.0f / FireRate))
	{
		return;
	}

	// 3. 탄약 확인
	if (CurrentAmmo <= 0)
	{
		Reload();
		return;
	}

	// 4. 총알 발사 로직
	if (BulletFactory)
	{
		// PlayerPawn에 있던 정적 오프셋 대신, 이제 WeaponMesh에 붙은 FirePosition의 실제 위치/회전을 사용합니다.
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
		SpawnParams.Instigator = Cast<APawn>(GetOwner()); // 무기를 소유한 플레이어를 Instigator로 설정

		ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(BulletFactory, SpawnLocation, SpawnRotation, SpawnParams);

		if (Bullet != nullptr)
		{
			// 필요하다면 총알에 데미지를 전달합니다.
			// Bullet->SetDamage(CalculatedDamage);

			CurrentAmmo--;
			LastFireTime = CurrentTime;
			UE_LOG(LogTemp, Log, TEXT("Fire! Ammo: %d / %d"), CurrentAmmo, MaxAmmo);

			// 발사 후 플레이어의 UI도 함께 갱신해주면 좋습니다.
			UpdatePlayerHUD();

			// 탄약이 방금 다 떨어졌다면 바로 재장전 시작
			if (CurrentAmmo <= 0)
			{
				Reload();
			}
		}
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
