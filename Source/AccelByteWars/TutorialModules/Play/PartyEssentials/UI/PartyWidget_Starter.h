// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Play/PartyEssentials/PartyEssentialsLog.h"
#include "PartyWidget_Starter.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UAccelByteWarsOnlineSessionBase;
class UPartyWidgetEntry_Starter;
class UCommonButtonBase;
class UHorizontalBox;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPartyWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void DisplayParty();
	void OnRetryButtonClicked();
	void OnLeaveButtonClicked();

	UPROPERTY(EditDefaultsOnly)
	int32 MaxPartyMembers = 4;

	UAccelByteWarsOnlineSessionBase* PartyOnlineSession;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Party;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_Party;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Leave;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPartyWidgetEntry_Starter> PartyWidgetEntryClass;
};
