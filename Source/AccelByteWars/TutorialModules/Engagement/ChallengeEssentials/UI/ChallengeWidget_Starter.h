// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Engagement/ChallengeEssentials/ChallengeEssentialsSubsystem_Starter.h"
#include "ChallengeWidget_Starter.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UListView;
class UTextBlock;
class UCommonButtonBase;

UCLASS()
class ACCELBYTEWARS_API UChallengeWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	EAccelByteModelsChallengeRotation Period = EAccelByteModelsChallengeRotation::NONE;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

#pragma region Module Challenge Essentials Function Declarations
	// TODO: Add your protected function declarations here.
#pragma endregion

	UPROPERTY()
	UChallengeEssentialsSubsystem_Starter* ChallengeEssentialsSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Challenge;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Challenge;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Title;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
};
