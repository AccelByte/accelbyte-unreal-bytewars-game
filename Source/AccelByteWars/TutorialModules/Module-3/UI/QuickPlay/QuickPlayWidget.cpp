// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-3/UI/QuickPlay/QuickPlayWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Components/WidgetSwitcher.h"
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

	SetQuickPlayState(EQuickPlayState::Default);

	Btn_Elimination->OnClicked().AddUObject(this, &UQuickPlayWidget::StartMatchmaking, FString::Printf(TEXT("Elimination")));
	Btn_TeamDeathmatch->OnClicked().AddUObject(this, &UQuickPlayWidget::StartMatchmaking, FString::Printf(TEXT("TeamDeathmatch")));

	Btn_Cancel->OnClicked().AddUObject(this, &UQuickPlayWidget::CancelMatchmaking);
	Btn_Ok->OnClicked().AddWeakLambda(this, [this]() { SetQuickPlayState(EQuickPlayState::Default); });
}

void UQuickPlayWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Elimination->OnClicked().Clear();
	Btn_TeamDeathmatch->OnClicked().Clear();

	Btn_Cancel->OnClicked().Clear();
	Btn_Ok->OnClicked().Clear();
}

void UQuickPlayWidget::StartMatchmaking(const FString MatchPool)
{
	SetQuickPlayState(EQuickPlayState::FindingMatch);

	ensure(MatchmakingSubsystem);
	MatchmakingSubsystem->StartMatchmaking(GetOwningPlayer(), MatchPool, FOnMatchmakingDelegate::CreateUObject(this, &UQuickPlayWidget::OnMatchmaking));
}

void UQuickPlayWidget::CancelMatchmaking() 
{
	SetQuickPlayState(EQuickPlayState::CancelingMatch);

	ensure(MatchmakingSubsystem);
	MatchmakingSubsystem->CancelMatchmaking(GetOwningPlayer());
}

void UQuickPlayWidget::OnMatchmaking(EMatchmakingState MatchmakingState)
{
	if (MatchmakingState == EMatchmakingState::MatchFound) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Match found and success to join the game session."));
		return;
	}

	// Display widget based on matchmaking state.
	SetQuickPlayState((EQuickPlayState)((int)MatchmakingState));
}

void UQuickPlayWidget::SetQuickPlayState(const EQuickPlayState NewState) 
{
	// Switch widget based on the new state.
	bIsBackHandler = false;
	Ws_QuickPlayState->SetActiveWidgetIndex((int)NewState);

	// Set user focus to certain button based on the new state.
	switch (NewState)
	{
		case EQuickPlayState::Default:
			bIsBackHandler = true;
			Btn_Elimination->SetUserFocus(GetOwningPlayer());
			break;
		case EQuickPlayState::FindingMatch:
			Btn_Cancel->SetUserFocus(GetOwningPlayer());
			break;
		case EQuickPlayState::Failed:
			Btn_Ok->SetUserFocus(GetOwningPlayer());
			break;
	}
}