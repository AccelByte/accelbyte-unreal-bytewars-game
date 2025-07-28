// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
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

// @@@SNIPSTART WalletBalanceWidget.h-private
// @@@MULTISNIP WalletSubsystem {"selectedLines": ["1", "6-7"]}
// @@@MULTISNIP Functions {"selectedLines": ["1-4"]}
private:
	void ShowBalance(bool bWasSuccessful, const FAccelByteModelsWalletInfo& Response) const;

	void UpdateBalance(const ECurrencyType CurrencyType);

	UPROPERTY()
	UWalletEssentialsSubsystem* WalletSubsystem;
// @@@SNIPEND

#pragma region "UI"
// @@@SNIPSTART WalletBalanceWidget.h-private-ui
// @@@MULTISNIP UI {"selectedLines": ["1", "7-8"]}
// @@@MULTISNIP CurrencyBalanceEntryMap {"selectedLines": ["1-2"]}
// @@@MULTISNIP CurrencyBalanceClass {"selectedLines": ["1", "4-5"]}
private:
	TMap<FString, UWalletBalanceWidgetEntry*> CurrencyBalanceEntryMap;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UWalletBalanceWidgetEntry> CurrencyBalanceClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_Root;
#pragma endregion
// @@@SNIPEND
};
