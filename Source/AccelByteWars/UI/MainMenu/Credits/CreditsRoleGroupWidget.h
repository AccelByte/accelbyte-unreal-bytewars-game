// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CreditsRoleGroupWidget.generated.h"

class UCommonTextBlock;
class UVerticalBox;

UCLASS()
class ACCELBYTEWARS_API UCreditsRoleGroupWidget : public UCommonUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_RoleName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_CreditRoleGroup;

public:
	void InitData(const FText& RoleName);
	void AddChild(UCommonUserWidget* InWidget);
	void ClearChildren();
};