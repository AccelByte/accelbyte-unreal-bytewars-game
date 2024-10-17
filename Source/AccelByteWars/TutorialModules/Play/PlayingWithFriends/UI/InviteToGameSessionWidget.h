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

// @@@SNIPSTART InviteToGameSessionWidget.h-private
// @@@MULTISNIP InviteToGameSessionUI {"selectedLines": ["1", "12-13"]}
// @@@MULTISNIP GetFriendDataFromParentWidget {"selectedLines": ["1", "3"]}
// @@@MULTISNIP Timer {"selectedLines": ["1", "15"]}
// @@@MULTISNIP Subsystem {"selectedLines": ["1", "8-9"]}
// @@@MULTISNIP InviteToSession {"selectedLines": ["1", "6"]}
private:
#pragma region "Helper"
	UFriendData* GetFriendDataFromParentWidget();
#pragma endregion 

	void InviteToSession();

	UPROPERTY()
	UPlayingWithFriendsSubsystem* Subsystem;

#pragma region "UI Related"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Invite;

	FTimerHandle InviteDelayTimerHandle;
#pragma endregion
// @@@SNIPEND
};
