// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-8/FriendsSubsystem.h"
#include "FriendDetailsWidget.generated.h"

class UTextBlock;
class UBorder;

UCLASS(Abstract)
class ACCELBYTEWARS_API UFriendDetailsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void InitData(UFriendData* FriendData);

protected:
	UFriendData* CachedFriendData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UBorder* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Presence;

	FSlateBrush DefaultAvatarBrush;
};