// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "TutorialModules/Module-12/ManagingFriendsSubsystem.h"
#include "BlockedPlayerWidgetEntry.generated.h"

class UImage;
class UTextBlock;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBlockedPlayerWidgetEntry : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void OnUnblockButtonClicked();

	UManagingFriendsSubsystem* ManagingFriendsSubsystem;
	UFriendData* CachedBlockedPlayerData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Unblock;

	FSlateBrush DefaultAvatarBrush;
};
