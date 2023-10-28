// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Monetization/StoreItemPurchase/StoreItemPurchaseModel.h"
#include "ItemPurchaseWidgetEntry.generated.h"

class UAccelByteWarsButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UItemPurchaseWidgetEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(const FString& InCurrencyCode, const FOnPurchaseClicked& OnClicked);
	FString GetCurrencyCode() const { return CurrencyCode; }

private:
	FString CurrencyCode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Purchase;
};
