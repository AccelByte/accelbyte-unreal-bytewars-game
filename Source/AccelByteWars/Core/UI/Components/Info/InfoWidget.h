// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InfoWidget.generated.h"

class UTextBlock;

UCLASS()
class ACCELBYTEWARS_API UInfoWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void RefreshUI();

protected:
	virtual void NativeConstruct() override;

private:
	bool GetUserInfo(FString& OutUserNickname, FString& OutUserId) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Username;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_UserId;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_ProjectInfo;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_BuildInfo;
};
