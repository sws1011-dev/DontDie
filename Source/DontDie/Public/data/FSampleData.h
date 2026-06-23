#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FSampleData.generated.h"

USTRUCT(BLueprintType, Blueprintable)
struct FSampleData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FSampleData()
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MyValue;
};
