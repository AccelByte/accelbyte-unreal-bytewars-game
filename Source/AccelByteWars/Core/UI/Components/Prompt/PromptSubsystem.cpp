// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/Prompt/Loading/LoadingWidget.h"
#include "Core/UI/Components/Prompt/Loading/ReconnectingWidget.h"
#include "Core/UI/Components/Prompt/PushNotification/PushNotificationWidget.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsMainMenuGameMode.h"

void UPromptSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetWorld()->GetGameInstance());
	ensureMsgf(GameInstance, TEXT("GameInstance is nullptr."));
}

void UPromptSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPromptSubsystem::ShowMessagePopUp(const FText Header, const FText Body)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget) 
	{
		return;
	}

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
	if (!BaseUIWidget) 
	{
		return;
	}

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultPopUpWidgetClass, [Header, Body, Type, Callback](UAccelByteWarsActivatableWidget& Widget)
	{
		if (UPopUpWidget* PopUp = Cast<UPopUpWidget>(&Widget)) 
		{
			PopUp->SetPopUpText(Header, Body);
			PopUp->SetPopUpType(Type);
			PopUp->SetDynamicCallback(Callback);
		}
	});
}

void UPromptSubsystem::ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDelegate Callback)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget) 
	{
		return;
	}

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultPopUpWidgetClass, [Header, Body, Type, Callback](UAccelByteWarsActivatableWidget& Widget)
	{
		if (UPopUpWidget* PopUp = Cast<UPopUpWidget>(&Widget)) 
		{
			PopUp->SetPopUpText(Header, Body);
			PopUp->SetPopUpType(Type);
			PopUp->SetCallback(Callback);
		}
	});
}

void UPromptSubsystem::ShowLoading(const FText LoadingMessage)
{
	// Abort if loading widget is already displayed.
	if (LoadingWidget) 
	{
		return;
	}
	
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget) 
	{
		return;
	}

	LoadingWidget = Cast<ULoadingWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultLoadingWidgetClass));
	if (!LoadingWidget) 
	{
		return;
	}

	LoadingWidget->OnDeactivated().AddWeakLambda(this, [this]()
	{
		LoadingWidget = nullptr;
	});
	LoadingWidget->SetLoadingMessage(LoadingMessage);
}

void UPromptSubsystem::HideLoading()
{
	// No need to hide, the loading widget is not displayed.
	if (!LoadingWidget) 
	{
		return;
	}

	LoadingWidget->DeactivateWidget();
}

void UPromptSubsystem::PushNotification(UPushNotification* Notification)
{
	// Notification can only be displayed on Main Menu level.
	if (!Cast<AAccelByteWarsMainMenuGameState>(GetWorld()->GetGameState()))
	{
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget) 
	{
		return;
	}

	if (UPushNotificationWidget* NotificationWidget = BaseUIWidget->GetPushNotificationWidget())
	{
		NotificationWidget->PushNotification(Notification);
	}
}

void UPromptSubsystem::PushNotification(const FText Message, const FString& IconImageURL, const bool bUseDefaultIconOnEmpty, const FText ActionButton1, const FText ActionButton2, const FText ActionButton3, FPushNotificationDynamicDelegate ActionButtonCallback)
{
	UPushNotification* Notification = NewObject<UPushNotification>();
	Notification->Message = Message;
	Notification->IconImageURL = IconImageURL;
	Notification->bUseDefaultIconOnEmpty = bUseDefaultIconOnEmpty;
	Notification->ActionButtonTexts.Add(ActionButton1);
	Notification->ActionButtonTexts.Add(ActionButton2);
	Notification->ActionButtonTexts.Add(ActionButton3);

	Notification->ActionButtonDynamicCallback = ActionButtonCallback;
	
	PushNotification(Notification);
}

void UPromptSubsystem::PushNotification(const FText& Message, const FString& IconImageURL, const bool bUseDefaultIconOnEmpty, const FText& ActionButton1, const FText& ActionButton2, const FText& ActionButton3, FPushNotificationDelegate ActionButtonCallback)
{
	UPushNotification* Notification = NewObject<UPushNotification>();
	Notification->Message = Message;
	Notification->IconImageURL = IconImageURL;
	Notification->bUseDefaultIconOnEmpty = bUseDefaultIconOnEmpty;
	Notification->ActionButtonTexts.Add(ActionButton1);
	Notification->ActionButtonTexts.Add(ActionButton2);
	Notification->ActionButtonTexts.Add(ActionButton3);

	Notification->ActionButtonCallback = ActionButtonCallback;

	PushNotification(Notification);
}