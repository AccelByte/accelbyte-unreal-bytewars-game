// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Models/AccelByteEcommerceModels.h"
#include "WalletBalanceWidget_Starter.generated.h"

class UWalletEssentialsSubsystem_Starter;
class UWalletBalanceWidgetEntry;

UCLASS(Abstract)
class ACCELBYTEWARS_API UWalletBalanceWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
#pragma region "Tutorial"
	// put your code here
#pragma endregion

	UPROPERTY()
	UWalletEssentialsSubsystem_Starter* WalletSubsystem;

	const TMap<FString, ECurrencyType> CurrenciesMap = {};

#pragma region "UI"
private:
	TMap<FString, UWalletBalanceWidgetEntry*> CurrencyBalanceEntryMap;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UWalletBalanceWidgetEntry> CurrencyBalanceClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_Root;
#pragma endregion 
};
