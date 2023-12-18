﻿// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Models/AccelByteEcommerceModels.h"
#include "WalletBalanceWidget.generated.h"

class UWalletEssentialsSubsystem;
class UWalletBalanceWidgetEntry;

UCLASS(Abstract)
class ACCELBYTEWARS_API UWalletBalanceWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

public:
	void UpdateBalance(const bool bAlwaysRequestToService);

protected:
	void ShowBalance(bool bWasSuccessful, const FAccelByteModelsWalletInfo& Response) const;

private:
	UPROPERTY()
	UWalletEssentialsSubsystem* WalletSubsystem;

	UPROPERTY(EditAnywhere)
	TArray<FString> Currencies = {
		"BC",
		"BG"
	};

#pragma region "UI Related"
private:
	TMap<FString, UWalletBalanceWidgetEntry*> CurrencyBalanceEntryMap;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UWalletBalanceWidgetEntry> CurrencyBalanceClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_Root;
#pragma endregion 
};