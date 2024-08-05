// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "SectionedShopWidget_Starter.h"

#include "Components/ListView.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Monetization/InGameStoreEssentials/UI/ShopWidget_Starter.h"

void USectionedShopWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	InGameStoreDisplaysSubsystem = GetGameInstance()->GetSubsystem<UInGameStoreDisplaysSubsystem_Starter>();
	ensure(InGameStoreDisplaysSubsystem);

#pragma region "Tutorial"
	// Put your code here
#pragma endregion

	// Bind parent's (store essentials) refresh function with this refresh function
	if (UShopWidget_Starter* ShopWidget = GetFirstOccurenceOuter<UShopWidget_Starter>())
	{
		ShopWidget->OnRefreshButtonClickedDelegates.AddUObject(this, &ThisClass::OnParentRefreshButtonClicked);
	}
}

void USectionedShopWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (UShopWidget_Starter* ShopWidget = GetFirstOccurenceOuter<UShopWidget_Starter>())
	{
		ShopWidget->OnRefreshButtonClickedDelegates.RemoveAll(this);
	}
}

#pragma region "Tutorial"
// Put your code here
#pragma endregion

#pragma region "UI"
FLinearColor USectionedShopWidget_Starter::GetSectionPresetColor(const int Index) const
{
	const int TrueIndex = AccelByteWarsUtility::PositiveModulo(Index, SectionBackgroundColorPreset.Num());
	return SectionBackgroundColorPreset[TrueIndex];
}

void USectionedShopWidget_Starter::OnParentRefreshButtonClicked()
{
	if (bRefreshing)
	{
		return;
	}
	bRefreshing = true;

	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
#pragma region "Tutorial"
	// Put your code here
#pragma endregion
}
#pragma endregion 
