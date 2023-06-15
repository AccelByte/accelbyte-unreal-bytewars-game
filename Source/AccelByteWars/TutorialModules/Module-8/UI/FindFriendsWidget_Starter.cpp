// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FindFriendsWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"
#include "Components/EditableText.h"

void UFindFriendsWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem_Starter>();
	ensure(FriendsSubsystem);
}

void UFindFriendsWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Edt_SearchBar->SetText(FText::FromString(TEXT("")));
	Edt_SearchBar->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchBarCommitted);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);
}

void UFindFriendsWidget_Starter::NativeOnDeactivated()
{
	Edt_SearchBar->OnTextCommitted.Clear();

	Super::NativeOnDeactivated();
}

void UFindFriendsWidget_Starter::OnSearchBarCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// TODO: Get and display find friend result here.
}