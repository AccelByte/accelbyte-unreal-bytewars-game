// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "Social/FriendsEssentials/FriendsSubsystem_Starter.h"
#include "FriendWidgetEntry_Starter.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;
class UTextBlock;
class UAccelByteWarsAsyncImageWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UFriendWidgetEntry_Starter : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void OnInviteButtonClicked();
	void OnAcceptButtonClicked();
	void OnRejectButtonClicked();
	void OnCancelButtonClicked();

	UFriendsSubsystem_Starter* FriendsSubsystem;
	UFriendData* CachedFriendData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsAsyncImageWidget* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_OptionButtons;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Invite;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_CannotInviteMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Accept;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Reject;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Cancel;
};