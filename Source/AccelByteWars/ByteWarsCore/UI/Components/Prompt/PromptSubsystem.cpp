// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/UI/Components/Prompt/PromptSubsystem.h"
#include "ByteWarsCore/UI/Components/Prompt/Loading/LoadingWidget.h"
#include "ByteWarsCore/UI/AccelByteWarsBaseUI.h"
#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"

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
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->BaseUIWidget);
	ensure(BaseUIWidget);

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultPopUpWidgetClass, [Header, Body](UAccelByteWarsActivatableWidget& Widget)
	{
		UPopUpWidget* PopUp = Cast<UPopUpWidget>(&Widget);
		PopUp->SetPopUpText(Header, Body);
		PopUp->SetPopUpType(EPopUpType::MessageOk);
	});
}

void UPromptSubsystem::ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDelegate Callback)
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->BaseUIWidget);
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
	
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->BaseUIWidget);
	ensure(BaseUIWidget);

	LoadingWidget = Cast<ULoadingWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Prompt, BaseUIWidget->DefaultLoadingWidgetClass));
	LoadingWidget->SetLoadingMessage(LoadingMessage);
}

void UPromptSubsystem::HideLoading()
{
	if (!LoadingWidget) 
	{
		return;
	}

	LoadingWidget->DeactivateWidget();
	LoadingWidget = nullptr;
}