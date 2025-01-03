// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CreateMatchSessionP2PWidget_Starter.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Play/MatchSessionEssentials/MatchSessionEssentialsModels.h"
#include "Play/MatchSessionEssentials/UI/CreateMatchSessionWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UCreateMatchSessionP2PWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Get online session
	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);
	ensure(OnlineSession);

	// Get parent menu widget
	W_Parent = GetFirstOccurenceOuter<UCreateMatchSessionWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	// TODO: Add your Online Session delegates setup here

	// TODO: Add your UI delegates setup here
}

void UCreateMatchSessionP2PWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// TODO: Add your Online Session delegates cleanup here

	// TODO: Add your UI delegates cleanup here
}

#pragma region "Match Session with P2P function implementations"
// TODO: Add your function implementations here
#pragma endregion 

#pragma region "UI related"
UWidget* UCreateMatchSessionP2PWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Btn_StartMatchSessionP2P;
}
#pragma endregion 
