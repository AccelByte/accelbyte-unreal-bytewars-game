// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsAssetManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AssetManagementSubsystem.generated.h"

class UAccelByteWarsAssetManager;
/**
 * 
 */
UCLASS(config = Game)
class ACCELBYTEWARS_API UAssetManagementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get ByteWars Asset Manager"))
	UAccelByteWarsAssetManager* GetByteWarsAssetManager() { return AssetManager; }

private:
	UPROPERTY(Transient)
	UAccelByteWarsAssetManager* AssetManager = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UAccelByteWarsAssetManager> AssetManagerClass;
};
