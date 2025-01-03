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
	
// @@@SNIPSTART PresenceWidget.h-protected
// @@@MULTISNIP PresenceUI {"selectedLines": ["1", "20-24"]}
// @@@MULTISNIP PresenceUserId {"selectedLines": ["1", "15"]}
// @@@MULTISNIP RefreshPresence {"selectedLines": ["1", "8"]}
// @@@MULTISNIP OnPresenceUpdated {"selectedLines": ["1", "9"]}
// @@@MULTISNIP OnBulkQueryPresenceComplete {"selectedLines": ["1", "10"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void SetupPresence();
	void OnSetupPresenceComplete();

	void RefreshPresence(bool bForceQueryPresence = false);
	void OnPresenceUpdated(const FUniqueNetId& UserId, const TSharedRef<FOnlineUserPresence>& Presence);
	void OnBulkQueryPresenceComplete(const bool bWasSuccessful, const FUserIDPresenceMap& Presences);

	UPROPERTY()
	UPresenceEssentialsSubsystem* PresenceEssentialsSubsystem;

	FUniqueNetIdPtr PresenceUserId;

	UPROPERTY()
	UListViewBase* ParentListView;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Tb_Presence;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UThrobber* Th_Loader;
// @@@SNIPEND
};
