// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
#include "AccelByteWarsAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	UAccelByteWarsAssetManager();
	static UAccelByteWarsAssetManager& Get();

protected:
	//~UAssetManager interface
	virtual void StartInitialLoading() override;
#if WITH_EDITOR
	//virtual void PreBeginPIE(bool bStartSimulate) override;
#endif
	//~End of UAssetManager interface

//////////////////////////////////////////////////////////////////////////
/// GameModes
//////////////////////////////////////////////////////////////////////////

public:
	static TArray<FGameModeData> GetAllGameModes();
	static TArray<FGameModeTypeData> GetAllGameModeTypes();
};
