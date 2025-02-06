// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingModels.h"
#include "CustomMatchmakingWidget_Starter.generated.h"

class UCustomMatchmakingSubsystem_Starter;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCustomMatchmakingWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
#pragma region "Tutorial"
	// Declare your functions here
#pragma endregion 

private:
	UPROPERTY()
	UCustomMatchmakingSubsystem_Starter* Subsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* W_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_StartMatchmaking;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	void SwitchWidget(EAccelByteWarsWidgetSwitcherState State);
};
