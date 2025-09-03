// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CustomMatchWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsSequentialSelectionWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Play/CustomMatch/CustomMatchSubsystem.h"
#include "Play/CustomMatch/CustomMatchModels.h"

void UCustomMatchWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	W_GameModeTypeSelection->OnSelectionChangedDelegate.AddUObject(this, &ThisClass::OnGameModeTypeSelectionChanged);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Back_Error->OnClicked().AddUObject(this, &ThisClass::ReturnToCreateMenu);

	SwitchContent(EContentType::CREATE);
	OnGameModeTypeSelectionChanged(W_GameModeTypeSelection->GetSelectedIndex());

	Btn_Create->OnClicked().AddUObject(this, &ThisClass::CreateCustomGameSession);
	Ws_Root->OnCancelClicked.AddUObject(this, &UCustomMatchWidget::LeaveGameSession);

	// Get subsystem
	Subsystem = GetGameInstance()->GetSubsystem<UCustomMatchSubsystem>();
	ensure(Subsystem);

	// Response delegates setup
	Subsystem->OnCreateCustomGameSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateGameSessionComplete);
	Subsystem->OnLeaveGameSessionDelegates.AddUObject(this, &ThisClass::OnLeaveGameSessionComplete);
	Subsystem->OnSessionServerUpdateReceivedDelegates.AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
}

void UCustomMatchWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	W_GameModeTypeSelection->OnSelectionChangedDelegate.RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Back_Error->OnClicked().RemoveAll(this);

	Btn_Create->OnClicked().RemoveAll(this);
	Ws_Root->OnCancelClicked.RemoveAll(this);

	// Response delegates cleanup
	Subsystem->OnCreateCustomGameSessionCompleteDelegates.RemoveAll(this);
	Subsystem->OnLeaveGameSessionDelegates.RemoveAll(this);
	Subsystem->OnSessionServerUpdateReceivedDelegates.RemoveAll(this);
}

void UCustomMatchWidget::CreateCustomGameSession()
{
	// Joinability. 0 = CLOSED, 1 = INVITE_ONLY, 2 = OPEN
	EAccelByteV2SessionJoinability Joinability = EAccelByteV2SessionJoinability::EMPTY;
	switch (W_JoinabilitySelection->GetSelectedIndex())
	{
	case 0:
		Joinability = EAccelByteV2SessionJoinability::CLOSED;
		break;
	case 1:
		Joinability = EAccelByteV2SessionJoinability::INVITE_ONLY;
		break;
	case 2:
		Joinability = EAccelByteV2SessionJoinability::OPEN;
		break;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}
	const int32 LocalUserNum = LocalPlayer->GetControllerId();

	Subsystem->CreateCustomGameSession(
		LocalUserNum,
		static_cast<EGameModeNetworkType>(W_NetworkTypeSelection->GetSelectedIndex()),
		static_cast<EGameModeType>(W_GameModeTypeSelection->GetSelectedIndex()),
		static_cast<EGameStyle>(W_GameStyleSelection->GetSelectedIndex()),
		Joinability,
		FCString::Atoi(*W_DurationSelection->GetSelected().ToString()),
		FCString::Atoi(*W_PlayerLivesSelection->GetSelected().ToString()),
		FCString::Atoi(*W_MissileLimitSelection->GetSelected().ToString()),
		FCString::Atoi(*W_MaxTotalPlayerSelection->GetSelected().ToString()),
		FCString::Atoi(*W_MaxTeamSelection->GetSelected().ToString()));

	Ws_Root->LoadingMessage = TEXT_REQUESTING_SESSION_CREATION;
	Ws_Root->bEnableCancelButton = false;
	SwitchContent(EContentType::LOADING);
}

void UCustomMatchWidget::OnCreateGameSessionComplete(const FName SessionName, const bool bSucceeded)
{
	if (bSucceeded)
	{
		Ws_Root->LoadingMessage = TEXT_JOINING_SESSION;
		Ws_Root->bEnableCancelButton = true;
		SwitchContent(EContentType::LOADING);
	}
	else
	{
		Ws_Root->ErrorMessage = TEXT_FAILED_TO_REQUEST_SESSION_CREATION;
		SwitchContent(EContentType::ERROR);
	}
}

void UCustomMatchWidget::LeaveGameSession()
{
	Subsystem->LeaveGameSession();

	Ws_Root->LoadingMessage = TEXT_LEAVING_SESSION;
	Ws_Root->bEnableCancelButton = false;
	SwitchContent(EContentType::LOADING);
}

void UCustomMatchWidget::OnLeaveGameSessionComplete(const FName SessionName, const bool bSucceeded)
{
	SwitchContent(EContentType::CREATE);
}

void UCustomMatchWidget::OnSessionServerUpdateReceived(
	const FName SessionName,
	const FOnlineError& Error,
	const bool bHasClientTravelTriggered)
{
	/* Keep showing the loading state with the cancel button until the client travels to the server.
	 * This is to allow the player to cancel the custom match session. */
	if (Error.bSucceeded && !bHasClientTravelTriggered)
	{
		Ws_Root->LoadingMessage = TEXT_JOINING_SESSION;
		Ws_Root->bEnableCancelButton = true;
		SwitchContent(EContentType::LOADING);
	}
	else if (!bHasClientTravelTriggered && !Error.bSucceeded)
	{
		Ws_Root->ErrorMessage = TEXT_FAILED_TO_JOIN_SESSION;
		SwitchContent(EContentType::ERROR);
	}
}

#pragma region "UI related"
void UCustomMatchWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 750.0f, 160.0f));
}

void UCustomMatchWidget::OnGameModeTypeSelectionChanged(int32 Index) const
{
	W_MaxTeamSelection->SetVisibility(static_cast<EGameModeType>(Index) == EGameModeType::TDM ?
		ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCustomMatchWidget::SwitchContent(const EContentType State)
{
	ESlateVisibility BackButtonVisibility;
	ESlateVisibility ErrorBackButtonVisibility;
	EAccelByteWarsWidgetSwitcherState WidgetSwitcherState;
	UWidget* FocusTarget;

	switch (State)
	{
	case EContentType::CREATE:
		BackButtonVisibility = ESlateVisibility::Visible;
		ErrorBackButtonVisibility = ESlateVisibility::Collapsed;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Not_Empty;
		FocusTarget = W_GameModeTypeSelection;
		break;
	case EContentType::LOADING:
		BackButtonVisibility = ESlateVisibility::Collapsed;
		ErrorBackButtonVisibility = ESlateVisibility::Collapsed;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Loading;
		FocusTarget = Ws_Root;
		break;
	case EContentType::ERROR:
		BackButtonVisibility = ESlateVisibility::Collapsed;
		ErrorBackButtonVisibility = ESlateVisibility::Visible;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Error;
		FocusTarget = Ws_Root;
		break;
	default:
		BackButtonVisibility = ESlateVisibility::Visible;
		ErrorBackButtonVisibility = ESlateVisibility::Collapsed;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Not_Empty;
		FocusTarget = W_GameModeTypeSelection;
	}

	Ws_Root->SetWidgetState(WidgetSwitcherState);
	Btn_Back_Error->SetVisibility(ErrorBackButtonVisibility);
	Btn_Back->SetVisibility(BackButtonVisibility);
	FocusTarget->SetUserFocus(GetOwningPlayer());
	bIsBackHandler = BackButtonVisibility == ESlateVisibility::Visible;
}

void UCustomMatchWidget::ReturnToCreateMenu()
{
	SwitchContent(EContentType::CREATE);
	LeaveGameSession();
}
#pragma endregion 
