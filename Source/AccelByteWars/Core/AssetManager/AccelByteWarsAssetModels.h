// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AccelByteWarsAssetModels.generated.h"

USTRUCT(BlueprintType)
struct FTutorialModuleData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
	TSubclassOf<class UAccelByteWarsActivatableWidget> DefaultUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsStarterModeActive = false;

#pragma region "Online Session"
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOnlineSessionModule = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UOnlineSession> OnlineSessionClass;
#pragma endregion 
};