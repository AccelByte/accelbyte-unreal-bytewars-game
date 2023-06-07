// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-3/UI/QuickPlayWidget_Starter.h"
#include "TutorialModules/Module-3/MatchmakingSubsystem_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UQuickPlayWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
}

void UQuickPlayWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetQuickPlayState(EMatchmakingState::Default);

	Btn_Elimination->OnClicked().AddUObject(this, &ThisClass::OnEliminationButtonClicked);
	Btn_TeamDeathmatch->OnClicked().AddUObject(this, &ThisClass::OnTeamDeathmatchButtonClicked);

	Btn_DedicatedServerType->OnClicked().AddUObject(this, &ThisClass::OnDedicatedServerTypeButtonClicked);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::OnCancelMatchmakingButtonClicked);

	Btn_Ok->OnClicked().AddWeakLambda(this, [this]() { SetQuickPlayState(EMatchmakingState::Default); });
}

void UQuickPlayWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Elimination->OnClicked().Clear();
	Btn_TeamDeathmatch->OnClicked().Clear();

	Btn_DedicatedServerType->OnClicked().Clear();
	Btn_Cancel->OnClicked().Clear();

	Btn_Ok->OnClicked().Clear();
}

void UQuickPlayWidget_Starter::SetQuickPlayState(const EMatchmakingState NewState)
{
	bIsBackHandler = false;
	uint8 StateIndex = (uint8)NewState;

	// Enable cancel matchmaking button only when start matchmaking is successful/finished.
	Btn_Cancel->SetIsEnabled(NewState == EMatchmakingState::FindingMatch);

	// Start match and find match should use the same UI.
	if (NewState >= EMatchmakingState::StartMatchmaking && NewState <= EMatchmakingState::FindingMatch)
	{
		StateIndex = (int8)EMatchmakingState::StartMatchmaking;
	}
	/* Since start match and find match state is considered as one,
	 * we need to substract any state that is greater than find match state. */
	else if (NewState > EMatchmakingState::FindingMatch)
	{
		StateIndex--;
	}

	// Switch widget based on the new state.
	Ws_QuickPlayState->SetActiveWidgetIndex(StateIndex);

	// Set user focus to certain button based on the new state.
	switch (NewState)
	{
	case EMatchmakingState::Default:
		bIsBackHandler = Ws_QuickPlayMenu->ActiveWidgetIndex == 0;
		if (bIsBackHandler)
		{
			Btn_Elimination->SetUserFocus(GetOwningPlayer());
		}
		else
		{
			Btn_DedicatedServerType->SetUserFocus(GetOwningPlayer());
		}
		break;
	case EMatchmakingState::FindingMatch:
		Btn_Cancel->SetUserFocus(GetOwningPlayer());
		break;
	case EMatchmakingState::FindMatchFailed:
		Btn_Ok->SetUserFocus(GetOwningPlayer());
		break;
	}
}

void UQuickPlayWidget_Starter::SetMatchGameMode(const FString& InMatchGameMode)
{
	MatchPoolGameMode = InMatchGameMode;

	// Switch to game server type selection.
	Ws_QuickPlayMenu->SetActiveWidgetIndex(1);
}

FString UQuickPlayWidget_Starter::GetMatchGameMode() const
{
	return MatchPoolGameMode;
}


#pragma region Module.3 General Function Definitions
void UQuickPlayWidget_Starter::OnEliminationButtonClicked()
{
	SetMatchGameMode(TEXT("elimination"));
}

void UQuickPlayWidget_Starter::OnTeamDeathmatchButtonClicked()
{
	SetMatchGameMode(TEXT("teamdeathmatch"));
}

void UQuickPlayWidget_Starter::OnDedicatedServerTypeButtonClicked()
{
	// TODO: Trigger start matchmaking using dedicated server here.
}

void UQuickPlayWidget_Starter::OnCancelMatchmakingButtonClicked()
{
	if (OnRequestCancelMatchmaking.IsBound())
	{
		OnRequestCancelMatchmaking.Broadcast(GetOwningPlayer());
	}

	OnRequestCancelMatchmaking.Clear();
}
#pragma endregion


#pragma region Module.3c Function Definitions

// TODO: Add your Module.3c function definitions here.

#pragma endregion