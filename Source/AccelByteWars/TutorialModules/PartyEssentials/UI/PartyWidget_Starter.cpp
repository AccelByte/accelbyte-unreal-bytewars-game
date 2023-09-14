// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/PartyEssentials/UI/PartyWidget_Starter.h"

#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "TutorialModules/SessionEssentials/SessionEssentialsModel.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "TutorialModules/PartyEssentials/UI/PartyWidgetEntry_Starter.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/HorizontalBox.h"
#include "CommonButtonBase.h"

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

void UPartyWidget_Starter::OnLeaveButtonClicked()
{
	// TODO: Call leave party here.
}