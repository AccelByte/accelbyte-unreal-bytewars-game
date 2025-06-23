// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Engagement/ChallengeEssentials/ChallengeEssentialsLog.h"
#include "Models/AccelByteChallengeModels.h"
#include "ChallengePeriodWidget.generated.h"

class UChallengeWidget;
class UCommonButtonBase;

UCLASS()
class ACCELBYTEWARS_API UChallengePeriodWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
// @@@SNIPSTART ChallengePeriodWidget.h-protected
// @@@MULTISNIP ChallengePeriodUI {"selectedLines": ["1", "8-15"]}
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void OpenChallengeMenu(const EAccelByteModelsChallengeRotation Period);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Alltime;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Daily;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Weekly;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UChallengeWidget> ChallengeWidgetClass;
// @@@SNIPEND
};
