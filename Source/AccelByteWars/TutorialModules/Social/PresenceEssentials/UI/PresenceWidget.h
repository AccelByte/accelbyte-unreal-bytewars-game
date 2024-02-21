// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/PresenceEssentials/PresenceEssentialsSubsystem.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PresenceWidget.generated.h"

class UCommonTextBlock;
class UThrobber;
class UListViewBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPresenceWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void SetupPresence();
	void OnSetupPresenceComplete();

	void RefreshPresence();
	void OnPresenceUpdated(const FUniqueNetId& UserId, const TSharedRef<FOnlineUserPresence>& Presence);
	void OnBulkQueryPresenceComplete(const bool bWasSuccessful, const FUserIDPresenceMap& Presences);

	UPresenceEssentialsSubsystem* PresenceEssentialsSubsystem;

	FUniqueNetIdPtr PresenceUserId;
	UListViewBase* ParentListView;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Tb_Presence;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UThrobber* Th_Loader;
};
