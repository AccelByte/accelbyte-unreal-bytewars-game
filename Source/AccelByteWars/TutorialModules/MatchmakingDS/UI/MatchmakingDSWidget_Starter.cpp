// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/MatchmakingDS/UI/MatchmakingDSWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"

#include "TutorialModules/MatchmakingEssentials/UI/QuickPlayWidget.h"
#include "TutorialModules/MatchmakingEssentials/MatchmakingEssentialsModels.h"
#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UMatchmakingDSWidget_Starter::NativeOnActivated()
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

void UMatchmakingDSWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// TODO: Unbind your delegates here.
}

UWidget* UMatchmakingDSWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Btn_StartMatchmakingDS;
}

#pragma region "Matchmaking with DS Function Definitions"

// TODO: Add your module function definitions here.

#pragma endregion 