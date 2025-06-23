// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Engagement/ChallengeEssentials/ChallengeEssentialsSubsystem.h"
#include "ChallengeWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UListView;
class UTextBlock;
class UCommonButtonBase;

UCLASS()
class ACCELBYTEWARS_API UChallengeWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
// @@@SNIPSTART ChallengeWidget.h-public
// @@@MULTISNIP ChallengePeriod {"selectedLines": ["1-2"]}
public:
	EAccelByteModelsChallengeRotation Period = EAccelByteModelsChallengeRotation::NONE;
// @@@SNIPEND

// @@@SNIPSTART ChallengeWidget.h-protected
// @@@MULTISNIP ChallengeUI {"selectedLines": ["1", "12-19"]}
// @@@MULTISNIP GetChallengeGoalList {"selectedLines": ["1", "7"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void GetChallengeGoalList();

	UPROPERTY()
	UChallengeEssentialsSubsystem* ChallengeEssentialsSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Challenge;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Challenge;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Title;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
// @@@SNIPEND
};
