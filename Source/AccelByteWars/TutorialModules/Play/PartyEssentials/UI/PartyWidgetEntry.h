// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Play/PartyEssentials/PartyEssentialsLog.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "PartyWidgetEntry.generated.h"

class UPanelWidget;
class UWidgetSwitcher;
class UCommonButtonBase;
class UPlayerEntryWidget;
class UFriendData;
class UAccelByteWarsOnlineSessionBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPartyWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void SetPartyMember(const FUserOnlineAccountAccelByte& PartyMember, const bool bIsLeader = false);
	void ResetPartyMember();

protected:
	virtual void NativeConstruct() override;
	
	void AddPartyMember();
	void OpenPlayerActionMenu();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPartyMemberColor(const FLinearColor MemberColor);

	UAccelByteWarsOnlineSessionBase* PartyOnlineSession;
	
	UPROPERTY()
	UFriendData* CachedFriendData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	FLinearColor PartyLeaderColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	FLinearColor PartyMemberColor;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_PartyMemberState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_PartyMemberPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UPlayerEntryWidget* W_PartyMember;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_PartyMember;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_AddPartyMember;

	UPROPERTY(EditDefaultsOnly, meta = (DisplayThumbnail = false))
	UTutorialModuleDataAsset* FriendsEssentialsModule;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "FriendsEssentialsModule!=nullptr", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> FriendsWidgetClass;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "FriendsEssentialsModule!=nullptr", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> FriendDetailsWidgetClass;
};
