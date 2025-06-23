// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "TutorialModules/Engagement/ChallengeEssentials/ChallengeEssentialsSubsystem_Starter.h"
#include "ChallengeWidgetEntry_Starter.generated.h"

class UPromptSubsystem;
class UWidgetSwitcher;
class UTextBlock;
class UCheckBox;
class UDynamicEntryBox;
class UHorizontalBox;
class UAccelByteWarsButtonBase;

UCLASS()
class ACCELBYTEWARS_API UChallengeWidgetEntry_Starter : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void OnClaimButtonClicked();

	UPROPERTY()
	UChallengeEssentialsSubsystem_Starter* ChallengeEssentialsSubsystem;

	UPROPERTY()
	UChallengeGoalData* GoalData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCheckBox* Cb_ChallengeStatus;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Goal;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_RemainingTime;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Progress;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_Progress;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Claim;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Progress;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UDynamicEntryBox* Deb_Reward;

	UPromptSubsystem* GetPromptSubystem();
};
