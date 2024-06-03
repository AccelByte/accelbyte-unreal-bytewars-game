// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Storage/StatisticsEssentials/StatsEssentialsLog.h"
#include "ProfileWidget.generated.h"

class UTextBlock;
class UCommonButtonBase;
class UAccelByteWarsAsyncImageWidget;

UCLASS()
class ACCELBYTEWARS_API UProfileWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnActivated() override;

	void ShowPlayerProfile();
	void CopyPlayerUserIdToClipboard();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsAsyncImageWidget* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_UserId;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_CopyUserId;
};
