// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PartyWidget_Starter.h"

#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "Play/SessionEssentials/SessionEssentialsModel.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "PartyWidgetEntry_Starter.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/HorizontalBox.h"
#include "CommonButtonBase.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

void UPartyWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PartyOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	ensure(PartyOnlineSession);
}

void UPartyWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Ws_Party->OnRetryClicked.Clear();
	Ws_Party->OnRetryClicked.AddUObject(this, &ThisClass::OnRetryButtonClicked);

	Btn_Leave->OnClicked().Clear();
	Btn_Leave->OnClicked().AddUObject(this, &ThisClass::OnLeaveButtonClicked);

	// TODO: Bind party event here.

	DisplayParty();
}

void UPartyWidget_Starter::NativeOnDeactivated()
{
	Btn_Leave->OnClicked().Clear();

	// TODO: Unbind party event here.

	Super::NativeOnDeactivated();
}

void UPartyWidget_Starter::DisplayParty()
{
	Btn_Leave->SetVisibility(ESlateVisibility::Collapsed);
	Ws_Party->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);

	// TODO: Query and display party member here.
}

void UPartyWidget_Starter::OnRetryButtonClicked()
{
	if (!PartyOnlineSession)
	{
		return;
	}

	const int32 LocalUserNum =
		PartyOnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer());

	Ws_Party->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	PartyOnlineSession->CreateParty(LocalUserNum);
}

void UPartyWidget_Starter::OnLeaveButtonClicked()
{
	// TODO: Call leave party here.
}