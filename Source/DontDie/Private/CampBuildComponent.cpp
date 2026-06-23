// Fill out your copyright notice in the Description page of Project Settings.


#include "CampBuildComponent.h"

#include "CampGridTile.h"
#include "PreviewActor.h"
#include "DrawDebugHelpers.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UCampBuildComponent::UCampBuildComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCampBuildComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCampBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBuildMode)
	{
		UpdateTileTrace();
	}
}

void UCampBuildComponent::ToggleBuildMode()
{
	SetBuildMode(!bBuildMode);
}

void UCampBuildComponent::SetBuildMode(bool bEnabled)
{
	bBuildMode = bEnabled;

	if (!bBuildMode)
	{
		SetCurrentTile(nullptr);
		DestroyPreviewActor();
	}
	else
	{
		SpawnPreviewActor();
	}
}

bool UCampBuildComponent::IsBuildModeEnabled() const
{
	return bBuildMode;
}

ACampGridTile* UCampBuildComponent::GetCurrentTile() const
{
	return CurrentTile;
}

void UCampBuildComponent::UpdateTileTrace()
{
	UWorld* World = GetWorld();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (World == nullptr || OwnerPawn == nullptr)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (PC == nullptr || PC->PlayerCameraManager == nullptr)
	{
		return;
	}

	const FVector TraceStart = PC->PlayerCameraManager->GetCameraLocation();
	const FVector TraceEnd = TraceStart + PC->PlayerCameraManager->GetCameraRotation().Vector() * TraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	ACampGridTile* HitTile = bHit ? Cast<ACampGridTile>(HitResult.GetActor()) : nullptr;

	SetCurrentTile(HitTile);
	UpdatePreviewActor();

	if (bDrawDebugTrace)
	{
		const FColor TraceColor = HitTile != nullptr ? FColor::Green : FColor::Red;
		DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, false, 0.0f, 0, 2.0f);

		if (bHit)
		{
			DrawDebugSphere(World, HitResult.ImpactPoint, 12.0f, 8, TraceColor, false, 0.0f);
		}
	}
}

void UCampBuildComponent::SetCurrentTile(ACampGridTile* NewTile)
{
	if (CurrentTile == NewTile)
	{
		return;
	}

	if (CurrentTile != nullptr)
	{
		CurrentTile->SetTileState(ECampGridTileState::Normal);
	}

	CurrentTile = NewTile;

	if (CurrentTile != nullptr)
	{
		CurrentTile->SetTileState(ECampGridTileState::Selected);
	}
}

void UCampBuildComponent::SpawnPreviewActor()
{
	if (PreviewActor != nullptr || PreviewActorClass == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	PreviewActor = World->SpawnActor<APreviewActor>(PreviewActorClass);
	if (PreviewActor != nullptr)
	{
		PreviewActor->SetPreviewSize(PreviewTileSize);
		PreviewActor->SetActorHiddenInGame(true);
	}
}

void UCampBuildComponent::DestroyPreviewActor()
{
	if (PreviewActor != nullptr)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UCampBuildComponent::UpdatePreviewActor()
{
	if (PreviewActor == nullptr)
	{
		return;
	}

	if (CurrentTile == nullptr)
	{
		PreviewActor->SetActorHiddenInGame(true);
		return;
	}

	const FVector PreviewLocation = CurrentTile->GetActorLocation() + FVector(0.0f, 0.0f, PreviewZOffset);
	PreviewActor->SetActorLocation(PreviewLocation);
	PreviewActor->SetActorHiddenInGame(false);
	PreviewActor->SetPreviewValid(true);
}
