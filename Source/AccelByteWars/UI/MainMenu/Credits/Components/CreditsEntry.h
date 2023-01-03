// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/MainMenu/Credits/Components/CreditsDataModel.h"
#include "CreditsEntry.generated.h"

class UCommonTextBlock;

UCLASS()
class ACCELBYTEWARS_API UCreditsEntry : public UCommonUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_Name;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_AdditionalDesc;

public:
	void InitData(const FCreditsData& CreditData);
};
