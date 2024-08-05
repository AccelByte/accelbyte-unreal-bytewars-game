// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CloudSaveLog.h"
#include "CloudSaveModels.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CoreMinimal.h"
#include "CloudSaveSubsystem.generated.h"

class UAccelByteWarsGameInstance;
class AAccelByteWarsPlayerPawn;
class AAccelByteWarsPlayerState;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UCloudSaveSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	FOnLoadPlayerEquipmentComplete OnLoadPlayerEquipmentCompleteDelegates;

#pragma region Module.5 Function Declarations
public:
	void SetPlayerRecord(
		const APlayerController* PlayerController,
		const FString& RecordKey,
		const FJsonObject& RecordData,
		const FOnSetCloudSaveRecordComplete& OnSetRecordComplete);
	void GetPlayerRecord(
		const APlayerController* PlayerController,
		const FString& RecordKey,
		const FOnGetCloudSaveRecordComplete& OnGetRecordComplete);
	void DeletePlayerRecord(
		const APlayerController* PlayerController,
		const FString& RecordKey,
		const FOnDeleteCloudSaveRecordComplete& OnDeleteRecordComplete);

private:
	TMultiMap<FString, FOnSetCloudSaveRecordComplete> SetPlayerRecordParams;
	TMultiMap<FString, FOnGetCloudSaveRecordComplete> GetPlayerRecordParams;
	TMultiMap<FString, FOnDeleteCloudSaveRecordComplete> DeletePlayerRecordParams;

	void OnSetPlayerRecordComplete(
		int32 LocalUserNum,
		const FOnlineError& Result,
		const FString& Key);
	void OnGetPlayerRecordComplete(
		int32 LocalUserNum,
		const FOnlineError& Result,
		const FString& Key,
		const FAccelByteModelsUserRecord& UserRecord);
	void OnDeletePlayerRecordComplete(
		int32 LocalUserNum,
		const FOnlineError& Result,
		const FString& Key);
#pragma endregion

private:
	void BindDelegates();
	void UnbindDelegates();

	void OnLoadGameSoundOptions(const APlayerController* PlayerController, TDelegate<void()> OnComplete);
	void OnSaveGameSoundOptions(const APlayerController* PlayerController, TDelegate<void()> OnComplete);

	void LoadPlayerEquipment(const APlayerController* PlayerController);
	void SavePlayersEquipment();

#pragma region "Utilities"
	
	int32 GetLocalUserIndex(const APlayerController* PlayerController) const;
#pragma endregion 

	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
};