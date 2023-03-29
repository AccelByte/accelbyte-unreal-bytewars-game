// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-3/UI/QuickPlayWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UQuickPlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	MatchmakingSubsystem = GameInstance->GetSubsystem<UMatchmakingEssentialsSubsystem>();
	ensure(MatchmakingSubsystem);
}

void UQuickPlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void UQuickPlayWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetQuickPlayState(EMatchmakingState::Default);

	Btn_Elimination->OnClicked().AddUObject(this, &ThisClass::OnEliminationButtonClicked);
	Btn_TeamDeathmatch->OnClicked().AddUObject(this, &ThisClass::OnTeamDeathmatchButtonClicked);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::OnCancelMatchmakingClicked);

	Btn_Ok->OnClicked().AddWeakLambda(this, [this]() { SetQuickPlayState(EMatchmakingState::Default); });
}

void UQuickPlayWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Elimination->OnClicked().Clear();
	Btn_TeamDeathmatch->OnClicked().Clear();

	Btn_Cancel->OnClicked().Clear();
	Btn_Ok->OnClicked().Clear();
}

void UQuickPlayWidget::SetQuickPlayState(const EMatchmakingState NewState)
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
		bIsBackHandler = true;
		Btn_Elimination->SetUserFocus(GetOwningPlayer());
		break;
	case EMatchmakingState::FindingMatch:
		Btn_Cancel->SetUserFocus(GetOwningPlayer());
		break;
	case EMatchmakingState::FindMatchFailed:
		Btn_Ok->SetUserFocus(GetOwningPlayer());
		break;
	}
}


#pragma region Module.3 General Function Definitions
void UQuickPlayWidget::OnEliminationButtonClicked()
{
	StartMatchmaking(TEXT("Elimination"));
}

void UQuickPlayWidget::OnTeamDeathmatchButtonClicked()
{
	StartMatchmaking(TEXT("TeamDeathmatch"));
}

void UQuickPlayWidget::OnCancelMatchmakingClicked()
{
	CancelMatchmaking();
}
#pragma endregion


#pragma region Module.3a Function Definitions
void UQuickPlayWidget::StartMatchmaking(const FString MatchPool)
{
	SetQuickPlayState(EMatchmakingState::FindingMatch);

	ensure(MatchmakingSubsystem);
	MatchmakingSubsystem->StartMatchmaking(GetOwningPlayer(), MatchPool, FOnMatchmakingStateChangedDelegate::CreateUObject(this, &ThisClass::OnMatchmaking));
}

void UQuickPlayWidget::CancelMatchmaking()
{
	SetQuickPlayState(EMatchmakingState::CancelingMatch);

	ensure(MatchmakingSubsystem);
	MatchmakingSubsystem->CancelMatchmaking(GetOwningPlayer());
}
#pragma endregion


#pragma region Module.3c Function Definitions
void UQuickPlayWidget::OnMatchmaking(EMatchmakingState MatchmakingState, FString ErrorMessage)
{
	if (MatchmakingState == EMatchmakingState::MatchFound)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Match found and success to join the game session."));
		return;
	}
	else if (MatchmakingState == EMatchmakingState::FindMatchFailed)
	{
		Tb_FailedMessage->SetText(FText::FromString(ErrorMessage));
	}

	// Display widget based on matchmaking state.
	SetQuickPlayState(MatchmakingState);
}
#pragma endregion