// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/UI/Components/Prompt/PopUp/PopUpWidget.h"
#include "PromptSubsystem.generated.h"

class UAccelByteWarsGameInstance;
class ULoadingWidget;

UCLASS()
class ACCELBYTEWARS_API UPromptSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Pop Up Prompt")
	void ShowMessagePopUp(const FText Header, const FText Body);

	/** Show dialogue pop-up with dynamic delegate. It is intended for Blueprint binding. */
	UFUNCTION(BlueprintCallable, Category = "Pop Up Prompt")
	void ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDynamicDelegate Callback);

	/** Show dialogue pop-up with non-dynamic delegate. It is intended for binding C++ function. */
	void ShowDialoguePopUp(const FText Header, const FText Body, const EPopUpType Type, FPopUpResultDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "Loading Prompt")
	void ShowLoading(const FText LoadingMessage = INVTEXT("Loading"));

	UFUNCTION(BlueprintCallable, Category = "Loading Prompt")
	void HideLoading();

private:
	UAccelByteWarsGameInstance* GameInstance;
	ULoadingWidget* LoadingWidget;
};