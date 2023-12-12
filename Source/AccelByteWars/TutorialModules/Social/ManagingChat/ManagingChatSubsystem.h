// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "ManagingChatLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "ManagingChatSubsystem.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UManagingChatSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

protected:
	void MuteChat(FUniqueNetIdPtr TargetUserId);
	void UnmuteChat(FUniqueNetIdPtr TargetUserId);

	void UpdateGeneratedWidgets();

	FUniqueNetIdPtr GetCurrentDisplayedFriendId();

	FTutorialModuleGeneratedWidget* MuteChatButtonMetadata;
	FTutorialModuleGeneratedWidget* UnmuteChatButtonMetadata;

	FOnlineIdentityAccelBytePtr GetIdentityInterface();
	FOnlineChatAccelBytePtr GetChatInterface();

	UPromptSubsystem* GetPromptSubystem();
};
