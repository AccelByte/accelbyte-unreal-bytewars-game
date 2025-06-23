// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeWidgetEntry_Starter.h"
#include "ChallengeGoalRewardWidgetEntry.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "Components/DynamicEntryBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/HorizontalBox.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

#include "Algo/ForEach.h"
#include "Algo/MaxElement.h"

void UChallengeWidgetEntry_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetGameInstance())
	{
		ChallengeEssentialsSubsystem = GetGameInstance()->GetSubsystem<UChallengeEssentialsSubsystem_Starter>();
	}
	ensure(ChallengeEssentialsSubsystem);

	Cb_ChallengeStatus->SetIsEnabled(false);
	Cb_ChallengeStatus->SetCheckedState(ECheckBoxState::Unchecked);

	Btn_Claim->OnClicked().Clear();
	Btn_Claim->OnClicked().AddUObject(this, &ThisClass::OnClaimButtonClicked);
}

void UChallengeWidgetEntry_Starter::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	// TODO: Add your code here.
}

void UChallengeWidgetEntry_Starter::OnClaimButtonClicked()
{
	// TODO: Add your code here.
}

UPromptSubsystem* UChallengeWidgetEntry_Starter::GetPromptSubystem()
{
	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		return GameInstance->GetSubsystem<UPromptSubsystem>();
	}

	return nullptr;
}
