// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "InviteToGameSessionWidget.generated.h"

class UCommonButtonBase;
class UPlayingWithFriendsSubsystem;
class UFriendData;

UCLASS(Abstract)
class ACCELBYTEWARS_API UInviteToGameSessionWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

#pragma region "Helper"
private:
	UFriendData* GetFriendDataFromParentWidget();
#pragma endregion 

private:
	void InviteToSession();

private:
	UPROPERTY()
	UPlayingWithFriendsSubsystem* Subsystem;

#pragma region "UI Related"
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Invite;

	FTimerHandle InviteDelayTimerHandle;
#pragma endregion 
};
