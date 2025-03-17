// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "QuickPlayWidget.h"

#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Play/MatchmakingEssentials/MatchmakingEssentialsLog.h"

// @@@SNIPSTART QuickPlayWidget.cpp-GetSelectedGameModeType
EGameModeType UQuickPlayWidget::GetSelectedGameModeType() const
{
	return SelectedGameModeType;
}
// @@@SNIPEND

void UQuickPlayWidget::SwitchContent(const EContentType State)
{
	bool bShowBackButton = true;
	UWidget* FocusTarget = Btn_SelectGameMode_Back;
	UWidget* WidgetTarget = W_SelectGameMode;
	UWidget* BackButtonTarget = Btn_SelectGameMode_Back;

	switch (State)
	{
	case EContentType::SELECTGAMEMODE:
		bShowBackButton = true;
		FocusTarget = Btn_Elimination;
		WidgetTarget = W_SelectGameMode;
		BackButtonTarget = Btn_SelectGameMode_Back;
		CameraTargetY = 600.0f;
		InitializeFTUEDialogues(true);
		break;
	case EContentType::SELECTSERVERTYPE:
		bShowBackButton = true;
		FocusTarget = Btn_SelectServerType_Back;
		WidgetTarget = W_SelectServerType;
		BackButtonTarget = Btn_SelectServerType_Back;
		CameraTargetY = 750.0f;
		break;
	}

	DesiredFocusTargetButton = BackButtonTarget;
	FocusTarget->SetUserFocus(GetOwningPlayer());
	Ws_ContentOuter->SetActiveWidget(WidgetTarget);

	BackButtonTarget->SetVisibility(bShowBackButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	RequestRefreshFocus();
}

void UQuickPlayWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Elimination->OnClicked().AddUObject(this, &ThisClass::SelectGameMode, EGameModeType::FFA);
	Btn_TeamDeathMatch->OnClicked().AddUObject(this, &ThisClass::SelectGameMode, EGameModeType::TDM);

	Btn_SelectGameMode_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_SelectServerType_Back->OnClicked().AddUObject(this, &ThisClass::SwitchContent, EContentType::SELECTGAMEMODE);

	SwitchContent(EContentType::SELECTGAMEMODE);
}

void UQuickPlayWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Elimination->OnClicked().RemoveAll(this);
	Btn_TeamDeathMatch->OnClicked().RemoveAll(this);

	Btn_SelectGameMode_Back->OnClicked().RemoveAll(this);
	Btn_SelectServerType_Back->OnClicked().RemoveAll(this);
}

void UQuickPlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, CameraTargetY, 160.0f));
}

UWidget* UQuickPlayWidget::NativeGetDesiredFocusTarget() const
{
	return DesiredFocusTargetButton;
}

void UQuickPlayWidget::SelectGameMode(EGameModeType GameModeType)
{
	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (ensure(BaseOnlineSession))
	{
		UAccelByteWarsOnlineSessionBase* OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);
		if(ensure(OnlineSession) && OnlineSession->ValidateToStartMatchmaking.IsBound())
		{
			// Check if any requirement to start matchmaking for the current selected game mode. If it doesn't fulfill the requirement, will not proceed to next step.
			// The failure detail, onscreen notification or any other action to inform players that they cannot proceed should be done by the validator. 
			bool bResult = OnlineSession->ValidateToStartMatchmaking.Execute(GameModeType);
			if(!bResult)
			{
				UE_LOG_MATCHMAKING(Warning, TEXT("Cannot start matchmaking for game mode %s"), GameModeType == EGameModeType::FFA ? TEXT("Elimination") : TEXT("Team Deathmatch"));
				return;
			}
		}
	}

	// Set game mode and select server type.
	SelectedGameModeType = GameModeType;
	SwitchContent(EContentType::SELECTSERVERTYPE);
}
