// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "SessionChatLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "SessionChatSubsystem_Starter.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API USessionChatSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();
	
protected:
	void PushChatRoomMessageReceivedNotification(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	FOnlineChatAccelBytePtr GetChatInterface() const;
	FOnlineSessionV2AccelBytePtr GetSessionInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubsystem() const;

#pragma region Module Session Chat Function Declarations
public:
	// TODO: Add your Module Session Chat public function declarations here.
protected:
	// TODO: Add your Module Session Chat protected function declarations here.
private:
	// TODO: Add your Module Session Chat private function declarations here.
#pragma endregion
};
