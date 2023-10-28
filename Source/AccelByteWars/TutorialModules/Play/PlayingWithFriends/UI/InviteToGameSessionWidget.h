// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "InviteToGameSessionWidget.generated.h"

class UCommonButtonBase;
class UPlayingWithFriendsSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API UInviteToGameSessionWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	void InviteToSession();

	UPROPERTY()
	UPlayingWithFriendsSubsystem* Subsystem;

	FTimerHandle InviteDelayTimerHandle;

#pragma region "UI Related"
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Invite;
#pragma endregion 
};
