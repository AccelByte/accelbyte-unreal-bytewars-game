// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingModels.h"
#include "CustomMatchmakingWidget.generated.h"

class UCustomMatchmakingSubsystem;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCustomMatchmakingWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UFUNCTION()
	void StartMatchmaking();

	UFUNCTION()
	void StopMatchmaking();

	void OnMatchmakingStarted();
	void OnServerInfoReceived();
	void OnMessageReceived(const FMatchmakerPayload& Payload);
	void OnMatchmakingFailed(const FString& ErrorMessage);

private:
	UPROPERTY()
	UCustomMatchmakingSubsystem* Subsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* W_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_StartMatchmaking;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	void SwitchWidget(EAccelByteWarsWidgetSwitcherState State);
};
