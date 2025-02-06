// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CustomMatchmakingWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingLog.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingSubsystem.h"

// @@@SNIPSTART CustomMatchmakingWidget.cpp-NativeOnActivated
// @@@MULTISNIP Back {"selectedLines": ["1-2", "30", "40"]}
// @@@MULTISNIP Subsystem {"selectedLines": ["1-2", "23-27", "40"]}
// @@@MULTISNIP Buttons {"selectedLines": ["1-2", "31-33", "40"]}
// @@@MULTISNIP PutItAllTogether {"selectedLines": ["1-2", "35-40"]}
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
	Subsystem->OnMatchmakingStoppedDelegates.AddUObject(this, &ThisClass::OnMatchmakingFailed);
}
// @@@SNIPEND

// @@@SNIPSTART CustomMatchmakingWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP Buttons {"selectedLines": ["1-2", "7-9", "16"]}
// @@@MULTISNIP PutItAllTogether {"selectedLines": ["1-2", "11-16"]}
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
// @@@SNIPEND

void UCustomMatchmakingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

// @@@SNIPSTART CustomMatchmakingWidget.cpp-StartMatchmaking
// @@@MULTISNIP Init {"selectedLines": ["1-5", "8"]}
// @@@MULTISNIP PutItAllTogether {"selectedLines": ["1-2", "7-8"]}
void UCustomMatchmakingWidget::StartMatchmaking()
{
	W_Root->LoadingMessage = FText::FromString(TEXT_LOADING_REQUEST);
	W_Root->bEnableCancelButton = false;
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);

	Subsystem->StartMatchmaking();
}
// @@@SNIPEND

// @@@SNIPSTART CustomMatchmakingWidget.cpp-StopMatchmaking
// @@@MULTISNIP Init {"selectedLines": ["1-4", "7"]}
// @@@MULTISNIP PutItAllTogether {"selectedLines": ["1-2", "6-7"]}
void UCustomMatchmakingWidget::StopMatchmaking()
{
	W_Root->LoadingMessage = FText::FromString(TEXT_LOADING_CANCEL);
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);

	Subsystem->StopMatchmaking();
}
// @@@SNIPEND

// @@@SNIPSTART CustomMatchmakingWidget.cpp-OnMatchmakingStarted
void UCustomMatchmakingWidget::OnMatchmakingStarted()
{
	W_Root->LoadingMessage = FText::FromString(TEXT_LOADING_FINDING_MATCH);
	W_Root->bEnableCancelButton = true;
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);
}
// @@@SNIPEND

// @@@SNIPSTART CustomMatchmakingWidget.cpp-OnMessageReceived
void UCustomMatchmakingWidget::OnMessageReceived(const FMatchmakerPayload& Payload)
{
	W_Root->LoadingMessage = FText::FromString(Payload.Message);
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Loading);
}
// @@@SNIPEND

// @@@SNIPSTART CustomMatchmakingWidget.cpp-OnMatchmakingFailed
void UCustomMatchmakingWidget::OnMatchmakingFailed(const FString& ErrorMessage)
{
	W_Root->ErrorMessage = FText::FromString(ErrorMessage);
	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Error);
}
// @@@SNIPEND

// @@@SNIPSTART CustomMatchmakingWidget.cpp-SwitchWidget
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
// @@@SNIPEND
