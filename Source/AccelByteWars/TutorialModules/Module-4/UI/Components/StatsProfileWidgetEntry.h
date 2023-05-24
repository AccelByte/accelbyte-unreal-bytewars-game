// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatsProfileWidgetEntry.generated.h"

class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UStatsProfileWidgetEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FText StatCode, FText StatValue) const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_StatName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_StatValue;
};
