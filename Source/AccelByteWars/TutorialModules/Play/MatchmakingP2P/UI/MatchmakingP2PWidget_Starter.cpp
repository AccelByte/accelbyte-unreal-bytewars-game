// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2PWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Play/MatchmakingEssentials/MatchmakingEssentialsModels.h"
#include "Play/MatchmakingEssentials/UI/QuickPlayWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UMatchmakingP2PWidget_Starter::NativeOnActivated()
{
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
		}
	}

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// TODO: Bind your delegates here.
}

void UMatchmakingP2PWidget_Starter::NativeOnDeactivated()
{
	Btn_Back->OnClicked().RemoveAll(this);

	// TODO: Unbind your delegates here.

	Super::NativeOnDeactivated();
}

void UMatchmakingP2PWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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
			// TODO: Call Join Session
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

#pragma region "Matchmaking with P2P Function Definitions"

// TODO: Add your module function definitions here.

#pragma endregion 

void UMatchmakingP2PWidget_Starter::ChangeWidgetState(const EWidgetState State)
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
		if (SessionInfo && SessionInfo->IsP2PMatchmaking())
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
