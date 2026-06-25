#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BuildingDataRow.generated.h"

class ABuildableActor;
class UStaticMesh;

USTRUCT(BlueprintType)
struct FBuildingDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CategoryID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BuildingTypeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PriorityTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Tier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RequiredBuildingID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText BaseFunction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText UpgradeFunction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<ABuildableActor> BuildActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float SizeX = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float SizeY = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float SizeZ = 100.0f;
};
