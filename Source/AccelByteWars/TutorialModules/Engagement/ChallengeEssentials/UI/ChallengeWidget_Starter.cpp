// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeWidget_Starter.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UChallengeWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetGameInstance()) 
	{
		ChallengeEssentialsSubsystem = GetGameInstance()->GetSubsystem<UChallengeEssentialsSubsystem_Starter>();
	}
	ensure(ChallengeEssentialsSubsystem);
}

void UChallengeWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	// TODO: Add your code here.
}

void UChallengeWidget_Starter::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UChallengeWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Lv_Challenge->GetListItems().IsEmpty() ? Cast<UWidget>(Btn_Back) : Cast<UWidget>(Lv_Challenge);
}
