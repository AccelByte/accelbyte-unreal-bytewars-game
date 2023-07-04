// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/Prompt/Loading/LoadingWidget.h"
#include "Core/UI/Components/Prompt/PushNotification/PushNotificationWidget.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/System/AccelByteWarsGameInstance.h"

void UPromptSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
}

void UPromptSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPromptSubsystem::ShowMessagePopUp(const FText Header, const FText Body)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultPopUpWidgetClass, [Header, Body](UAccelByteWarsActivatableWidget& Widget)
	{
		UPopUpWidget* PopUp = Cast<UPopUpWidget>(&Widget);
		PopUp->SetPopUpText(Header, Body);
		PopUp->SetPopUpType(EPopUpType::MessageOk);
	});
}

void UPromptSubsystem::ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDynamicDelegate Callback)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultPopUpWidgetClass, [Header, Body, Type, Callback](UAccelByteWarsActivatableWidget& Widget)
	{
		UPopUpWidget* PopUp = Cast<UPopUpWidget>(&Widget);
		PopUp->SetPopUpText(Header, Body);
		PopUp->SetPopUpType(Type);
		PopUp->SetDynamicCallback(Callback);
	});
}

void UPromptSubsystem::ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDelegate Callback)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultPopUpWidgetClass, [Header, Body, Type, Callback](UAccelByteWarsActivatableWidget& Widget)
	{
		UPopUpWidget* PopUp = Cast<UPopUpWidget>(&Widget);
		PopUp->SetPopUpText(Header, Body);
		PopUp->SetPopUpType(Type);
		PopUp->SetCallback(Callback);
	});
}

void UPromptSubsystem::ShowLoading(const FText LoadingMessage)
{
	if (LoadingWidget) 
	{
		return;
	}
	
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	LoadingWidget = Cast<ULoadingWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultLoadingWidgetClass));
	LoadingWidget->OnDeactivated().AddWeakLambda(this, [this]()
	{
		LoadingWidget = nullptr;
	});
	LoadingWidget->SetLoadingMessage(LoadingMessage);
}

void UPromptSubsystem::HideLoading()
{
	if (!LoadingWidget) 
	{
		return;
	}

	LoadingWidget->DeactivateWidget();
}

void UPromptSubsystem::PushNotification(const FString& IconImageURL, const FText Message, const FText ActionButton1, const FText ActionButton2, const FText ActionButton3, FPushNotificationDynamicDelegate ActionButtonCallback)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	UPushNotification* Notification = NewObject<UPushNotification>();
	Notification->IconImageURL = IconImageURL;
	Notification->Message = Message;
	Notification->ActionButtonDynamicCallback = ActionButtonCallback;

	Notification->ActionButtonTexts.Add(ActionButton1);
	Notification->ActionButtonTexts.Add(ActionButton2);
	Notification->ActionButtonTexts.Add(ActionButton3);

	BaseUIWidget->GetPushNotificationWidget()->PushNotification(Notification);
}

void UPromptSubsystem::PushNotification(const FString& IconImageURL, const FText& Message, const FText& ActionButton1, const FText& ActionButton2, const FText& ActionButton3, FPushNotificationDelegate ActionButtonCallback)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	UPushNotification* Notification = NewObject<UPushNotification>();
	Notification->IconImageURL = IconImageURL;
	Notification->Message = Message;
	Notification->ActionButtonCallback = ActionButtonCallback;

	Notification->ActionButtonTexts.Add(ActionButton1);
	Notification->ActionButtonTexts.Add(ActionButton2);
	Notification->ActionButtonTexts.Add(ActionButton3);

	BaseUIWidget->GetPushNotificationWidget()->PushNotification(Notification);
}