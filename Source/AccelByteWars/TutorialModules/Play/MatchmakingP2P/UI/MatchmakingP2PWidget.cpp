// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2PWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Play/MatchmakingEssentials/MatchmakingEssentialsModels.h"
#include "Play/MatchmakingEssentials/UI/QuickPlayWidget.h"
#include "Play/MatchmakingP2P/MatchmakingP2PLog.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

// @@@SNIPSTART MatchmakingP2PWidget.cpp-NativeOnActivated
// @@@MULTISNIP OnlineSession {"selectedLines": ["1-2", "5-12", "42"]}
// @@@MULTISNIP SelectedGameModeType {"selectedLines": ["1-2", "15-23", "42"]}
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "25-29", "41-42"]}
void UMatchmakingP2PWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}

	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);
	ensure(OnlineSession);

	// Get selected game mode type from the previous widget
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsGameInstance>(GetGameInstance())->GetBaseUIWidget();
	for (const UCommonActivatableWidget* Widget : BaseUIWidget->Stacks[EBaseUIStackType::Menu]->GetWidgetList())
	{
		if (const UQuickPlayWidget* QuickPlayWidget = Cast<UQuickPlayWidget>(Widget))
		{
			SelectedGameModeType = QuickPlayWidget->GetSelectedGameModeType();
			SelectedGameStyle = QuickPlayWidget->GetSelectedGameStyle();
		}
	}

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Join->OnClicked().AddUObject(this, &ThisClass::JoinSession);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::CancelMatchmaking);
	Btn_Reject->OnClicked().AddUObject(this, &ThisClass::RejectSessionInvite);
	Btn_Retry->OnClicked().AddUObject(this, &ThisClass::StartMatchmaking);

	OnlineSession->GetOnStartMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnStartMatchmakingComplete);
	OnlineSession->GetOnMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnMatchmakingComplete);
	OnlineSession->GetOnSessionInviteReceivedDelegates()->AddUObject(this, &ThisClass::OnSessionInviteReceived);
	OnlineSession->GetOnJoinSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnJoinSessionComplete);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);

	OnlineSession->GetOnCancelMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	OnlineSession->GetOnRejectSessionInviteCompleteDelegate()->AddUObject(this, &ThisClass::OnRejectSessionInviteComplete);

	// Start matchmaking immediately
	StartMatchmaking();
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-7", "18-19"]}
void UMatchmakingP2PWidget::NativeOnDeactivated()
{
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Join->OnClicked().RemoveAll(this);
	Btn_Cancel->OnClicked().RemoveAll(this);
	Btn_Reject->OnClicked().RemoveAll(this);
	Btn_Retry->OnClicked().RemoveAll(this);

	OnlineSession->GetOnStartMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionInviteReceivedDelegates()->RemoveAll(this);
	OnlineSession->GetOnJoinSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->RemoveAll(this);

	OnlineSession->GetOnCancelMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnRejectSessionInviteCompleteDelegate()->RemoveAll(this);

	Super::NativeOnDeactivated();
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-NativeTick
// @@@MULTISNIP WaitingForPlayerStateCountdown {"selectedLines": ["1-14", "16-17", "44"]}
void UMatchmakingP2PWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 800.0f, 160.0f));

	// Manual "Auto" Join
	if (WidgetState == EWidgetState::WAITING_FOR_PLAYER && AutoJoinCurrentCountdown > 0)
	{
		AutoJoinCurrentCountdown -= InDeltaTime;
		Tb_WaitingForPlayersCountdown->SetText(FText::FromString(FString::FromInt(AutoJoinCurrentCountdown)));

		if (AutoJoinCurrentCountdown <= 0)
		{
			JoinSession();
		}
	}

	// Match found delay
	if (WidgetState == EWidgetState::MATCH_FOUND && MatchFoundCurrentCountdown > 0)
	{
		MatchFoundCurrentCountdown -= InDeltaTime;
		if (MatchFoundCurrentCountdown <= 0 && SessionInvite)
		{
			// Check if auto join is enabled or not
			const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(
				SessionInvite->Session.Session.SessionInfo);
			check(SessionInfo.IsValid());
			const bool bAutoJoin = SessionInfo->GetBackendSessionData()->Configuration.AutoJoin;

			ChangeWidgetState(bAutoJoin ? EWidgetState::JOINING_MATCH : EWidgetState::WAITING_FOR_PLAYER);
		}
	}

	// Session joined delay
	if (WidgetState == EWidgetState::SESSION_JOINED && SessionJoinedCurrentCountdown > 0)
	{
		SessionJoinedCurrentCountdown -= InDeltaTime;
		if (SessionJoinedCurrentCountdown <= 0)
		{
			ChangeWidgetState(EWidgetState::REQUESTING_SERVER);
		}
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-StartMatchmaking
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-18", "25"]}
void UMatchmakingP2PWidget::StartMatchmaking()
{
	if (OnlineSession->ValidateToStartMatchmaking.IsBound() && 
		!OnlineSession->ValidateToStartMatchmaking.Execute(SelectedGameModeType))
	{
		return;
	}

	// Reset stored invite
	SessionInvite = nullptr;

	// Reset auto join session countdown.
	AutoJoinCurrentCountdown = AutoJoinDelay;
	MatchFoundCurrentCountdown = MatchFoundDelay;
	SessionJoinedCurrentCountdown = SessionJoinedDelay;
	Tb_WaitingForPlayersCountdown->SetText(FText::FromString(FString::FromInt(AutoJoinCurrentCountdown)));

	ChangeWidgetState(EWidgetState::REQUEST_SENT);

	OnlineSession->StartMatchmaking(
		GetOwningPlayer(),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		EGameModeNetworkType::P2P,
		SelectedGameModeType, SelectedGameStyle);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-JoinSession
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-9", "15"]}
void UMatchmakingP2PWidget::JoinSession()
{
	if (!SessionInvite)
	{
		ChangeWidgetState(EWidgetState::ERROR);
		Tb_ErrorText->SetText(TEXT_FAILED_SESSION_INVITE_INVALID);
	}

	ChangeWidgetState(EWidgetState::JOINING_MATCH);

	OnlineSession->JoinSession(
		OnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		SessionInvite->Session);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-CancelMatchmaking
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-3", "8"]}
void UMatchmakingP2PWidget::CancelMatchmaking()
{
	ChangeWidgetState(EWidgetState::CANCELING_MATCH);

	OnlineSession->CancelMatchmaking(
		GetOwningPlayer(),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-RejectSessionInvite
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-9", "14"]}
void UMatchmakingP2PWidget::RejectSessionInvite()
{
	if (!SessionInvite)
	{
		ChangeWidgetState(EWidgetState::ERROR);
		Tb_ErrorText->SetText(TEXT_FAILED_SESSION_INVITE_INVALID);
	}
	
	ChangeWidgetState(EWidgetState::REJECTING_MATCH);

	OnlineSession->RejectSessionInvite(
		OnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()),
		*SessionInvite.Get());
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-OnStartMatchmakingComplete
void UMatchmakingP2PWidget::OnStartMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		ChangeWidgetState(EWidgetState::FINDING_MATCH);
	}
	else
	{
		Tb_ErrorText->SetText(TEXT_FAILED_FIND_MATCH);
		ChangeWidgetState(EWidgetState::ERROR);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-OnMatchmakingComplete
void UMatchmakingP2PWidget::OnMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		ChangeWidgetState(EWidgetState::MATCH_FOUND);
	}
	else
	{
		Tb_ErrorText->SetText(TEXT_FAILED_FIND_MATCH);
		ChangeWidgetState(EWidgetState::ERROR);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-OnSessionInviteReceived
void UMatchmakingP2PWidget::OnSessionInviteReceived(
	const FUniqueNetId& UserId,
	const FUniqueNetId& FromId,
	const FOnlineSessionInviteAccelByte& Invite)
{
	// Abort if not a game session.
	if (Invite.SessionType != EAccelByteV2SessionType::GameSession)
	{
		return;
	}

	// Store session invite for later use
	SessionInvite = MakeShared<FOnlineSessionInviteAccelByte>(Invite);

	// Check if auto join is enabled or not
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo =
		StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInvite->Session.Session.SessionInfo);
	check(SessionInfo.IsValid());
	const bool bAutoJoin = SessionInfo->GetBackendSessionData()->Configuration.AutoJoin;

	/**
	 * If auto join, show joining match screen, else show waiting for players screen.
	 * Only if the match found screen has been up for longer than MatchFoundDelay
	 */
	if (MatchFoundCurrentCountdown <= 0)
	{
		ChangeWidgetState(bAutoJoin ? EWidgetState::JOINING_MATCH : EWidgetState::WAITING_FOR_PLAYER);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-OnJoinSessionComplete
void UMatchmakingP2PWidget::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if(Result == EOnJoinSessionCompleteResult::Success)
	{
		ChangeWidgetState(EWidgetState::SESSION_JOINED);
	}
	else
	{
		Tb_ErrorText->SetText(TEXT_FAILED_JOIN_MATCH);
		ChangeWidgetState(EWidgetState::ERROR);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-OnSessionServerUpdateReceived
void UMatchmakingP2PWidget::OnSessionServerUpdateReceived(
	const FName SessionName,
	const FOnlineError& Error,
	const bool bHasClientTravelTriggered)
{
	if (Error.bSucceeded && SessionJoinedCurrentCountdown <= 0)
	{
		ChangeWidgetState(EWidgetState::REQUESTING_SERVER);
	}
	else if (!Error.bSucceeded)
	{
		Tb_ErrorText->SetText(TEXT_FAILED_FIND_SERVER);
		ChangeWidgetState(EWidgetState::ERROR);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-OnCancelMatchmakingComplete
void UMatchmakingP2PWidget::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	// Abort if match already found.
	if (WidgetState == EWidgetState::MATCH_FOUND || WidgetState == EWidgetState::WAITING_FOR_PLAYER)
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("Match already found, cancel matchmaking canceled."))
		return;
	}

	if (bSucceeded)
	{
		DeactivateWidget();
	}
	else
	{
		Tb_ErrorText->SetText(TEXT_FAILED_CANCEL_MATCH);
		ChangeWidgetState(EWidgetState::ERROR);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.cpp-OnRejectSessionInviteComplete
void UMatchmakingP2PWidget::OnRejectSessionInviteComplete(bool bSucceeded)
{
	if (bSucceeded)
	{
		DeactivateWidget();
	}
	else
	{
		Tb_ErrorText->SetText(TEXT_FAILED_REJECT_MATCH);
		ChangeWidgetState(EWidgetState::ERROR);
	}
}
// @@@SNIPEND

void UMatchmakingP2PWidget::ChangeWidgetState(const EWidgetState State)
{
	UWidget* WidgetSwitcherTarget = nullptr;
	UWidget* FocusTarget = nullptr;

	switch (State)
	{
	case EWidgetState::REQUEST_SENT:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingText->SetText(TEXT_FINDING_MATCH);
		Tb_LoadingSubText->SetVisibility(ESlateVisibility::Hidden);
		Btn_Cancel->SetIsEnabled(false);
		break;
	case EWidgetState::FINDING_MATCH:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingText->SetText(TEXT_FINDING_MATCH);
		Tb_LoadingSubText->SetVisibility(ESlateVisibility::Hidden);
		Btn_Cancel->SetIsEnabled(true);
		FocusTarget = Btn_Cancel;
		break;
	case EWidgetState::MATCH_FOUND:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingText->SetText(TEXT_FINDING_MATCH);
		Tb_LoadingSubText->SetVisibility(ESlateVisibility::Visible);
		Tb_LoadingSubText->SetText(TEXT_MATCH_FOUND);
		Btn_Cancel->SetIsEnabled(false);
		MatchFoundCurrentCountdown = MatchFoundDelay;
		break;
	case EWidgetState::CANCELING_MATCH:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingText->SetText(TEXT_CANCEL_MATCHMAKING);
		Btn_Cancel->SetIsEnabled(false);
		break;
	case EWidgetState::WAITING_FOR_PLAYER:
		WidgetSwitcherTarget = W_WaitingForPlayer;
		Btn_Cancel->SetIsEnabled(false);
		FocusTarget = Btn_Join;
		break;
	case EWidgetState::REJECTING_MATCH:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingText->SetText(TEXT_REJECTING_MATCH);
		Tb_LoadingSubText->SetVisibility(ESlateVisibility::Hidden);
		Btn_Cancel->SetIsEnabled(false);
		break;
	case EWidgetState::JOINING_MATCH:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingText->SetText(TEXT_JOINING_MATCH);
		Tb_LoadingSubText->SetVisibility(ESlateVisibility::Hidden);
		Btn_Cancel->SetIsEnabled(false);
		break;
	case EWidgetState::SESSION_JOINED:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingText->SetText(TEXT_JOINING_MATCH);
		Tb_LoadingSubText->SetVisibility(ESlateVisibility::Visible);
		Tb_LoadingSubText->SetText(TEXT_MATCH_JOINED);
		Btn_Cancel->SetIsEnabled(false);
		SessionJoinedCurrentCountdown = SessionJoinedDelay;
		break;
	case EWidgetState::REQUESTING_SERVER:
		WidgetSwitcherTarget = W_Loading;
		Tb_LoadingSubText->SetVisibility(ESlateVisibility::Hidden);
		Btn_Cancel->SetIsEnabled(false);
		break;
	case EWidgetState::ERROR:
		WidgetSwitcherTarget = W_Error;
		FocusTarget = Btn_Retry;
		break;
	}

	// Requesting server loading state based on P2P or DS and Host or Client
	if (State == EWidgetState::REQUESTING_SERVER)
	{
		const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo =
			StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInvite->Session.Session.SessionInfo);
		if (SessionInfo && SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P)
		{
			Tb_LoadingText->SetText(
				OnlineSession->GetLocalPlayerUniqueNetId(GetOwningPlayer()) == SessionInfo->GetLeaderId()?
				TEXT_STARTING_AS_HOST:
				TEXT_WAITING_HOST);
		}
		else
		{
			Tb_LoadingText->SetText(TEXT_REQUESTING_SERVER);
		}
	}

	if (FocusTarget)
	{
		FocusTarget->SetUserFocus(GetOwningPlayer());
	}
	Ws_Root->SetActiveWidget(WidgetSwitcherTarget);
	WidgetState = State;
}
