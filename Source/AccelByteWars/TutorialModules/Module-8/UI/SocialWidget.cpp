// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/SocialWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

void USocialWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USocialWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_FindFriends->OnClicked().AddUObject(this, &ThisClass::OpenFriendMenu, FindFriendsWidgetClass);
	Btn_Friends->OnClicked().AddUObject(this, &ThisClass::OpenFriendMenu, FriendsWidgetClass);
	Btn_FriendRequests->OnClicked().AddUObject(this, &ThisClass::OpenFriendMenu, FriendRequestsWidgetClass);
	Btn_SentFriendRequests->OnClicked().AddUObject(this, &ThisClass::OpenFriendMenu, SentFriendRequestsWidgetClass);
}

void USocialWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_FindFriends->OnClicked().Clear();
	Btn_Friends->OnClicked().Clear();
	Btn_FriendRequests->OnClicked().Clear();
	Btn_SentFriendRequests->OnClicked().Clear();
}

void USocialWidget::OpenFriendMenu(TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, WidgetClass);
}