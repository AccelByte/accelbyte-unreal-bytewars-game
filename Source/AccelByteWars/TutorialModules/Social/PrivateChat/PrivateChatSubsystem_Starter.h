// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "PrivateChatLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "PrivateChatSubsystem_Starter.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPrivateChatSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();
	
protected:
	void PushPrivateChatMessageReceivedNotification(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message);

	FOnlineChatAccelBytePtr GetChatInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubsystem() const;

#pragma region Module Private Chat Function Declarations
public:
	// TODO: Add your Module Private Chat public function declarations here.
	protected:
	// TODO: Add your Module Private Chat protected function declarations here.
	private:
	// TODO: Add your Module Private Chat private function declarations here.
#pragma endregion
};
