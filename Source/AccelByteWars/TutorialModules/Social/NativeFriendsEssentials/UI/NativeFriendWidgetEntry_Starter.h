// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "Social/FriendsEssentials/FriendsSubsystem_Starter.h"
#include "Social/NativeFriendsEssentials/NativeFriendsSubsystem_Starter.h"
#include "NativeFriendWidgetEntry_Starter.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;
class UTextBlock;
class UAccelByteWarsAsyncImageWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UNativeFriendWidgetEntry_Starter : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()

public:
	UNativeFriendData* GetNativeCachedFriendData() const { return NativeCachedFriendData; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void OnInviteButtonClicked();

	UPROPERTY()
	UFriendsSubsystem_Starter* FriendsSubsystem;

	UPROPERTY()
	UNativeFriendsSubsystem_Starter* NativeFriendsSubsystem;

	UPROPERTY()
	UNativeFriendData* NativeCachedFriendData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsAsyncImageWidget* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Option;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Message;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Invite;
};
