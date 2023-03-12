// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-3/UI/QuickPlayWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"

void UQuickPlayWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	MatchmakingSubsystem = GameInstance->GetSubsystem<UMatchmakingEssentialsSubsystem>();
	ensure(MatchmakingSubsystem);
}

void UQuickPlayWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void UQuickPlayWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetQuickPlayState(EMatchmakingState::Default);

	Btn_Elimination->OnClicked().AddUObject(this, &ThisClass::OnEliminationButtonClicked);
	Btn_TeamDeathmatch->OnClicked().AddUObject(this, &ThisClass::OnTeamDeathmatchButtonClicked);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::OnCancelMatchmakingClicked);

	Btn_Ok->OnClicked().AddWeakLambda(this, [this]() { SetQuickPlayState(EMatchmakingState::Default); });
}

void UQuickPlayWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Elimination->OnClicked().Clear();
	Btn_TeamDeathmatch->OnClicked().Clear();

	Btn_Cancel->OnClicked().Clear();
	Btn_Ok->OnClicked().Clear();
}

void UQuickPlayWidget_Starter::SetQuickPlayState(const EMatchmakingState NewState)
{
	// Switch widget based on the new state.
	bIsBackHandler = false;
	Ws_QuickPlayState->SetActiveWidgetIndex((int)NewState);

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

void UQuickPlayWidget_Starter::OnEliminationButtonClicked()
{
	// TODO: Call start matchmaking with Elimination game mode request here.
}

void UQuickPlayWidget_Starter::OnTeamDeathmatchButtonClicked()
{
	// TODO: Call start matchmaking with Team Deathmatch game mode request here.
}

void UQuickPlayWidget_Starter::OnCancelMatchmakingClicked()
{
	// TODO: Call cancel matchmaking request here.
}

#pragma endregion


#pragma region Module.3a Function Definitions

// TODO: Add your Module.3a function definitions here.

#pragma endregion


#pragma region Module.3c Function Definitions

// TODO: Add your Module.3c function definitions here.

#pragma endregion