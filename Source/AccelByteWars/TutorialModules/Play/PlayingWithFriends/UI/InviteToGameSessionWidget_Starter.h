// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "InviteToGameSessionWidget_Starter.generated.h"

class UCommonButtonBase;
class UPlayingWithFriendsSubsystem_Starter;
class UFriendData;

UCLASS(Abstract)
class ACCELBYTEWARS_API UInviteToGameSessionWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

#pragma region "Helper"
private:
	UFriendData* GetFriendDataFromParentWidget();
#pragma endregion

#pragma region "Playing with Friends function declaration"
private:
	// TODO: Add your private function declaration here
#pragma endregion 

private:
	UPROPERTY()
	UPlayingWithFriendsSubsystem_Starter* Subsystem;

#pragma region "UI Related"
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Invite;

	FTimerHandle InviteDelayTimerHandle;
#pragma endregion 
};
