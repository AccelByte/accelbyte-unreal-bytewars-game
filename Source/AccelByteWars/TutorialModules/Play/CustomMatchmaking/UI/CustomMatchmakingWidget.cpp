// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CustomMatchmakingWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingLog.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingSubsystem.h"

void UCustomMatchmakingWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

#pragma region "FTUE"
	if (FFTUEDialogueModel* FTUEModel = FFTUEDialogueModel::GetMetadataById("ftue_set_address", FTUEDialogues))
	{
		const FString LaunchArgument = FString::Printf(TEXT("-%s=\"<ip>:<port>\""), *CUSTOM_MATCHMAKING_CONFIG_KEY_URL);

		FServiceArgumentModel ArgumentModel;
		ArgumentModel.Argument = LaunchArgument;
		FTUEModel->MessageArguments.Add(ArgumentModel);

		FTUEModel->Button1.ButtonActionDelegate.AddWeakLambda(this, [LaunchArgument]()
		{
			FPlatformMisc::ClipboardCopy(*LaunchArgument);
		});
	}
#pragma endregion 

	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Not_Empty);

	Subsystem = GetGameInstance()->GetSubsystem<UCustomMatchmakingSubsystem>();
	if (!Subsystem)
	{
		UE_LOG_CUSTOMMATCHMAKING(Fatal, TEXT("Can't retrieve UCustomMatchmakingSubsystem"))
	}

	// Bind button
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_StartMatchmaking->OnClicked().AddUObject(this, &ThisClass::StartMatchmaking);
	W_Root->OnCancelClicked.AddUObject(this, &ThisClass::StopMatchmaking);
	W_Root->OnRetryClicked.AddUObject(this, &ThisClass::StartMatchmaking);

	// Bind events
	Subsystem->OnMatchmakingStartedDelegates.AddUObject(this, &ThisClass::OnMatchmakingStarted);
	Subsystem->OnMatchmakingErrorDelegates.AddUObject(this, &ThisClass::OnMatchmakingFailed);
	Subsystem->OnMatchmakingMessageReceivedDelegates.AddUObject(this, &ThisClass::OnMessageReceived);
	Subsystem->OnMatchmakingStoppedDelegates.AddWeakLambda(this, [this](const FString& Reason)
	{
		OnMatchmakingFailed(TEXT_ERROR_CANCELED);
	});
}

void UCustomMatchmakingWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// Unbind button
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_StartMatchmaking->OnClicked().RemoveAll(this);
	W_Root->OnCancelClicked.RemoveAll(this);
	W_Root->OnRetryClicked.RemoveAll(this);

	// Unbind events
	Subsystem->OnMatchmakingStartedDelegates.RemoveAll(this);
	Subsystem->OnMatchmakingErrorDelegates.RemoveAll(this);
	Subsystem->OnMatchmakingMessageReceivedDelegates.RemoveAll(this);
	Subsystem->OnMatchmakingStoppedDelegates.RemoveAll(this);
}

void UCustomMatchmakingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void UCustomMatchmakingWidget::StartMatchmaking()
{
	W_Root->LoadingMessage = FText::FromString(TEXT_LOADING_REQUEST);
	W_Root->bEnableCancelButton = false;
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);

	Subsystem->StartMatchmaking();
}

void UCustomMatchmakingWidget::StopMatchmaking()
{
	W_Root->LoadingMessage = FText::FromString(TEXT_LOADING_CANCEL);
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);

	Subsystem->StopMatchmaking();
}

void UCustomMatchmakingWidget::OnMatchmakingStarted()
{
	W_Root->LoadingMessage = FText::FromString(TEXT_LOADING_FINDING_MATCH);
	W_Root->bEnableCancelButton = true;
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);
}

void UCustomMatchmakingWidget::OnServerInfoReceived()
{
	W_Root->LoadingMessage = FText::FromString(TEXT_LOADING_TRAVELLING);
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);
}

void UCustomMatchmakingWidget::OnMessageReceived(const FMatchmakerPayload& Payload)
{
	W_Root->LoadingMessage = FText::FromString(Payload.Message);
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);
}

void UCustomMatchmakingWidget::OnMatchmakingFailed(const FString& ErrorMessage)
{
	FString ModifiableMessage = ErrorMessage;

	// Modify error message telling the player what to check
	if (ModifiableMessage.Contains(WEBSOCKET_FAILED_GENERIC_MESSAGE, ESearchCase::IgnoreCase) || ModifiableMessage.IsEmpty())
	{
		ModifiableMessage = TEXT_WEBSOCKET_ERROR_GENERIC;
	}

	W_Root->ErrorMessage = FText::FromString(ModifiableMessage);
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Error);
}

void UCustomMatchmakingWidget::SwitchWidget(const EAccelByteWarsWidgetSwitcherState State)
{
	UWidget* FocusTarget = W_Root;
	bool bIsBackable = true;

	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		bIsBackable = false;
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		FocusTarget = Btn_StartMatchmaking;
		break;
	}

	W_Root->SetWidgetState(State);

	FocusTarget->SetUserFocus(GetOwningPlayer());

	bIsBackHandler = bIsBackable;
	Btn_Back->SetVisibility(bIsBackable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	W_Root->ForceRefresh();
}
