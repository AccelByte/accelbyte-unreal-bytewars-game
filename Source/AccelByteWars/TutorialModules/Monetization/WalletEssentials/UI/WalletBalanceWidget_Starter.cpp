// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletBalanceWidget_Starter.h"

#include "Components/PanelWidget.h"
#include "Components/WalletBalanceWidgetEntry.h"
#include "Monetization/WalletEssentials/WalletEssentialsSubsystem_Starter.h"

void UWalletBalanceWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	WalletSubsystem = GetGameInstance()->GetSubsystem<UWalletEssentialsSubsystem_Starter>();
	ensure(WalletSubsystem);

#pragma region "Tutorial"
	// put your code here
#pragma endregion 
}

void UWalletBalanceWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

#pragma region "Tutorial"
	// put your code here
#pragma endregion 
}
