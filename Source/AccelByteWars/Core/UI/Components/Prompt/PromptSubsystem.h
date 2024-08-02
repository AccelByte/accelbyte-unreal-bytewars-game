// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/UI/Components/Prompt/PopUp/PopUpWidget.h"
#include "Core/UI/Components/Prompt/PushNotification/PushNotificationModels.h"
#include "PromptSubsystem.generated.h"

#define MESSAGE_PROMPT_TEXT NSLOCTEXT("AccelByteWars", "Message", "Message")
#define ERROR_PROMPT_TEXT NSLOCTEXT("AccelByteWars", "Error", "Error")

class UAccelByteWarsGameInstance;
class ULoadingWidget;
class UReconnectingWidget;

UCLASS()
class ACCELBYTEWARS_API UPromptSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Prompt Subsystem")
	void ShowMessagePopUp(const FText Header, const FText Body);

	/* Show dialogue pop-up with dynamic delegate (Blueprint). */
	UFUNCTION(BlueprintCallable, Category = "Prompt Subsystem")
	void ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDynamicDelegate Callback);

	/* Show dialogue pop-up with non-dynamic delegate (C++). */
	void ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "Prompt Subsystem")
	void ShowLoading(const FText LoadingMessage = NSLOCTEXT("AccelByteWars", "Loading", "Loading"));

	UFUNCTION(BlueprintCallable, Category = "Prompt Subsystem")
	void HideLoading();

	void PushNotification(UPushNotification* Notification);

	/* Push a notification (Blueprint). */
	UFUNCTION(BlueprintCallable)
	void PushNotification(const FText Message, const FString& IconImageURL, const bool bUseDefaultIconOnEmpty, const FText ActionButton1, const FText ActionButton2, const FText ActionButton3, FPushNotificationDynamicDelegate ActionButtonCallback);

	/* Push a notification (C++). */
	void PushNotification(const FText& Message, const FString& IconImageURL = FString(""), const bool bUseDefaultIconOnEmpty = false, const FText& ActionButton1 = FText::FromString(""), const FText& ActionButton2 = FText::FromString(""), const FText& ActionButton3 = FText::FromString(""), FPushNotificationDelegate ActionButtonCallback = FPushNotificationDelegate());

private:
	UAccelByteWarsGameInstance* GameInstance;
	ULoadingWidget* LoadingWidget;
};