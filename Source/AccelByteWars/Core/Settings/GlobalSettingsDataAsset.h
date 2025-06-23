// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalSettingsDataAsset.generated.h"

class UGameModeDataAssets;

USTRUCT(BlueprintType)
struct FGlobalTeamSetup {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor itemColor = FLinearColor::White;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ACCELBYTEWARS_API UGlobalSettingsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGlobalTeamSetup> GlobalTeamSetup;
};
