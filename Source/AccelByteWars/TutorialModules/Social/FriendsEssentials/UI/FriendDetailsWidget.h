// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "FriendDetailsWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS(Abstract)
class ACCELBYTEWARS_API UFriendDetailsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void InitData(UFriendData* FriendData);

	UFriendData* GetCachedFriendData() const
	{ 
		return CachedFriendData; 
	}

protected:
	UFriendData* CachedFriendData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Presence;

	FSlateBrush DefaultAvatarBrush;
};