// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "Social/ManagingFriends/ManagingFriendsSubsystem_Starter.h"
#include "BlockedPlayerWidgetEntry_Starter.generated.h"

class UImage;
class UTextBlock;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBlockedPlayerWidgetEntry_Starter : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void OnUnblockButtonClicked();

	UManagingFriendsSubsystem_Starter* ManagingFriendsSubsystem;
	UFriendData* CachedBlockedPlayerData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Unblock;

	FSlateBrush DefaultAvatarBrush;
};
