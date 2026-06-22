// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Bullet.h"
#include "player/PlayerCharacter.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


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
    if (CurrentAmmo <= 0 || RemainingBurstCount <= 0)
    {
       RemainingBurstCount = 0;
       GetWorldTimerManager().ClearTimer(BurstTimerHandle);
       if (CurrentAmmo <= 0) Reload();
       return;
    }

    if (BulletFactory)
    {
       FVector SpawnLocation = FirePosition->GetComponentLocation();
       FRotator SpawnRotation = FRotator::ZeroRotator;
       
       // ---------------- [에임 정중앙 조준 로직 추가] ----------------
       APlayerController* PC = nullptr;
       if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
       {
           PC = Cast<APlayerController>(OwnerPawn->GetController());
       }

       if (PC != nullptr && PC->PlayerCameraManager != nullptr)
       {
           // 1. 카메라의 현재 위치와 정면 방향 벡터를 가져옵니다.
           FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
           FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

           // 2. 화면 중앙 에임이 도달할 가상의 끝점 (충분히 먼 거리: 10,000 유닛)
           FVector TraceEnd = CameraLocation + (CameraForward * 10000.0f);

           FHitResult HitResult;
           FCollisionQueryParams TraceParams;
           TraceParams.AddIgnoredActor(this);          // 무기 자신 무시
           TraceParams.AddIgnoredActor(GetOwner());    // 플레이어 무시

           // 3. 카메라 정중앙에서 레이저(Line Trace)를 쏩니다.
           FVector TargetTargetLocation = TraceEnd;
           if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, TraceParams))
           {
               // 무언가(벽, 적)에 부딪혔다면 그 충돌 지점을 타겟으로 잡습니다.
               TargetTargetLocation = HitResult.ImpactPoint;
           }

           // 4. [핵심] 총구 위치에서 레이저가 부딪힌 정중앙 타겟 지점을 바라보는 회전 각도를 계산합니다!
           SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetTargetLocation);
       }
       else
       {
           // 컨트롤러가 없는 AI 등의 예외 상황엔 기존 총구 방향 처리
           SpawnRotation = FirePosition->GetComponentRotation();
       }
       // -------------------------------------------------------------

       FActorSpawnParameters SpawnParams;
       SpawnParams.Owner = this;
       SpawnParams.Instigator = Cast<APawn>(GetOwner());

       ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(BulletFactory, SpawnLocation, SpawnRotation, SpawnParams);

       if (Bullet != nullptr)
       {
          if (FireSound)
          {
             float TargetDuration = 1.0f / FireRate;
             float SoundDuration = FireSound->GetDuration();
             float Pitch = (SoundDuration > TargetDuration) ? (SoundDuration / TargetDuration) : 1.0f;
             UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, Pitch, FireSoundStartTime);
          }

          Bullet->BulletDamage = BaseDamage * CurrentBurstMultiplier;
          CurrentAmmo--;
          RemainingBurstCount--;
          
          UpdatePlayerHUD();
       }
    }

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

	// 재장전 사운드 재생
	if (ReloadSound)
	{
		// 재장전 시간(ReloadSpeed)에 맞게 피치 계산
		float SoundDuration = ReloadSound->GetDuration();
		
		// 피치 = 원본 소리 길이 / 목표 재장전 시간
		float Pitch = SoundDuration / ReloadSpeed;

		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, Pitch, ReloadSoundStartTime);
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
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

	APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
	if (Player)
	{
		Player->UpdateReloadingHUD(false);
	}
	
	UpdatePlayerHUD();
}

void AWeapon::UpdatePlayerHUD()
{
	// 이 무기의 소유자(Owner)를 플레이어 Pawn으로 캐스팅합니다.
	APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
	if (Player != nullptr)
	{
		// 플레이어에게 현재 남은 탄약 정보를 넘겨주며 UI 갱신을 요청합니다.
		Player->UpdateAmmoHUD(CurrentAmmo, MaxAmmo);
	}
}
