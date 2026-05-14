// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"

#include "Bullet.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
APlayerPawn::APlayerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("My Box Component"));

	SetRootComponent(BoxComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("My StaticMesh Component"));
	MeshComp->SetupAttachment(BoxComp);

	FVector boxSize = FVector(50.0f, 50.0f, 50.0f);
	BoxComp->SetBoxExtent(boxSize);

	FirePosition = CreateDefaultSubobject<UArrowComponent>(TEXT("Fire Component"));
	FirePosition->SetupAttachment(BoxComp);

	BoxComp->SetCollisionProfileName(TEXT("Player"));
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	CurrentAmmo = MaxAmmo;

	PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* subsys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (subsys != nullptr)
		{
			subsys->AddMappingContext(ImcPlayerInput, 0);
		}
	}
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector dir = FVector(MoveInput.Y, MoveInput.X, 0);
	dir.Normalize();

	FVector vector = dir * MoveSpeed * DeltaTime;

	SetActorLocation(GetActorLocation() + FVector(vector.X, 0, 0), true);
	SetActorLocation(GetActorLocation() + FVector(0, vector.Y, 0), true);

	if (PlayerController != nullptr)
	{
		FVector WorldLocation, WorldDirection;

		if (PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			float Distance = (GetActorLocation().Z - WorldLocation.Z) / WorldDirection.Z;
			FVector IntersectLocation = WorldLocation + (WorldDirection * Distance);

			FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), IntersectLocation);

			SetActorRotation(TargetRotation);
		}
	}
}

void APlayerPawn::OnInputMove(const struct FInputActionValue& value)
{
	MoveInput = value.Get<FVector2D>();
}

void APlayerPawn::Fire()
{
	// 1. 재장전 중인지 확인
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reloading..."));
		return;
	}

	// 2. 공격 속도(연사 제한) 확인
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < (1.0f / AttackSpeed))
	{
		return;
	}

	// 3. 탄약 확인
	if (CurrentAmmo <= 0)
	{
		Reload();
		return;
	}

	// 4. 발사 로직
	ABullet* bullet = GetWorld()->SpawnActor<ABullet>(BulletFactory,
	                                                  FirePosition->GetComponentLocation(),
	                                                  FirePosition->GetComponentRotation());

	if (bullet != nullptr)
	{
		CurrentAmmo--;
		LastFireTime = CurrentTime;
		UE_LOG(LogTemp, Log, TEXT("Fire! Ammo: %d / %d"), CurrentAmmo, MaxAmmo);

		// 탄약이 방금 다 떨어졌다면 바로 재장전 시작
		if (CurrentAmmo <= 0)
		{
			Reload();
		}
	}
}

void APlayerPawn::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo) return;

	bIsReloading = true;
	UE_LOG(LogTemp, Warning, TEXT("Reload Started... Wait %f sec"), ReloadSpeed);

	// 재장전 시간 후 완료 함수 호출
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &APlayerPawn::OnReloadComplete, ReloadSpeed, false);
}

void APlayerPawn::OnReloadComplete()
{
	bIsReloading = false;
	CurrentAmmo = MaxAmmo;
	UE_LOG(LogTemp, Warning, TEXT("Reload Complete! Ammo: %d"), CurrentAmmo);
}

float APlayerPawn::GetCalculatedDamage()
{
	float FinalDamage = AttackPower;

	if (FMath::FRand() < CritChance)
	{
		FinalDamage *= 2.0f;
		UE_LOG(LogTemp, Warning, TEXT("Critical Hit! Damage: %f"), FinalDamage);
	}

	return FinalDamage;
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* eic = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (eic != nullptr)
	{
		eic->BindAction(IaMove, ETriggerEvent::Triggered, this, &APlayerPawn::OnInputMove);
		eic->BindAction(IaMove, ETriggerEvent::Completed, this, &APlayerPawn::OnInputMove);
		eic->BindAction(IaFire, ETriggerEvent::Started, this, &APlayerPawn::Fire);
	}
}
