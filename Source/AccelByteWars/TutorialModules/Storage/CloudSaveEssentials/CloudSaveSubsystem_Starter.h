// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CloudSaveLog.h"
#include "CloudSaveModels.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CloudSaveSubsystem_Starter.generated.h"

class UAccelByteWarsGameInstance;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UCloudSaveSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

#pragma region Module.5 Function Declarations
public:
	// TODO: Add your public Module.5 function declarations here.

private:
	// TODO: Add your private Module.5 function declarations here.
#pragma endregion

private:
	void BindDelegates();
	void UnbindDelegates();

	void OnLoadGameSoundOptions(const APlayerController* PlayerController, TDelegate<void()> OnComplete);
	void OnSaveGameSoundOptions(const APlayerController* PlayerController, TDelegate<void()> OnComplete);

	FDelegateHandle OnSetPlayerRecordCompletedDelegateHandle;
	FDelegateHandle OnGetPlayerRecordCompletedDelegateHandle;
	FDelegateHandle OnDeletePlayerRecordCompletedDelegateHandle;

	TMultiMap<FString, FOnSetCloudSaveRecordComplete> SetPlayerRecordParams;
	TMultiMap<FString, FOnGetCloudSaveRecordComplete> GetPlayerRecordParams;
	TMultiMap<FString, FOnDeleteCloudSaveRecordComplete> DeletePlayerRecordParams;

#pragma region "Utilities"
	int32 GetLocalUserIndex(const APlayerController* PlayerController) const;
#pragma endregion 

	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
};