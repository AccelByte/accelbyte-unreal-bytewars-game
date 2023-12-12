// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2PWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"

#include "Play/MatchmakingEssentials/MatchmakingEssentialsModels.h"
#include "Play/MatchmakingEssentials/UI/QuickPlayWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UMatchmakingP2PWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}

	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);
	ensure(OnlineSession);

	W_Parent = GetFirstOccurenceOuter<UQuickPlayWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	// TODO: Bind your delegates here.
}

void UMatchmakingP2PWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// TODO: Unbind your delegates here.
}

UWidget* UMatchmakingP2PWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Btn_StartMatchmakingP2P;
}

#pragma region "Matchmaking with P2P Function Definitions"

// TODO: Add your module function definitions here.

#pragma endregion 
