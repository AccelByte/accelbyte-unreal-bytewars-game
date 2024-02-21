// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FindFriendsWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/Throbber.h"
#include "CommonButtonBase.h"
#include "HAL/PlatformApplicationMisc.h"

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
	Btn_CopyFriendCode->OnClicked().AddUObject(this, &ThisClass::CopyFriendCodeToClipboard);

	// Reset widgets.
	Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_FindFriends->ClearListItems();
}

void UFindFriendsWidget_Starter::NativeOnDeactivated()
{
	Edt_SearchBar->OnTextCommitted.Clear();
	Btn_CopyFriendCode->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

void UFindFriendsWidget_Starter::OnSearchBarCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// TODO: Get and display find friend result here.
}

void UFindFriendsWidget_Starter::CopyFriendCodeToClipboard()
{
	FPlatformApplicationMisc::ClipboardCopy(*Tb_FriendCode->GetText().ToString());
}