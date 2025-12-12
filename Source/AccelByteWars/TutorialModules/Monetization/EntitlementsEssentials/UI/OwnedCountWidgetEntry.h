// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "OwnedCountWidgetEntry.generated.h"

class UStoreItemListEntry;
class UTextBlock;
class UEntitlementsEssentialsSubsystem;
class UStoreItemDataObject;

UCLASS(Abstract)
class ACCELBYTEWARS_API UOwnedCountWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

// @@@SNIPSTART OwnedCountWidgetEntry.h-private
// @@@MULTISNIP Subsystem {"selectedLines": ["1-3"]}
// @@@MULTISNIP CheckItemEquipped {"selectedLines": ["1", "5-6"]}
// @@@MULTISNIP Setup {"selectedLines": ["1", "5-6"]}
private:
	UPROPERTY()
	UEntitlementsEssentialsSubsystem* EntitlementsSubsystem;

	void RetrieveEntitlementWithForceRequest(const bool bForceRequest);
	void ShowOwnedCount(const FOnlineError& Error, const UStoreItemDataObject* Entitlement);
// @@@SNIPEND

#pragma region "UI"
// @@@SNIPSTART OwnedCountWidgetEntry.h-private-ui
// @@@MULTISNIP Parent {"selectedLines": ["1-3"]}
// @@@MULTISNIP Setup {"selectedLines": ["1", "5-6"]}
private:
	UPROPERTY()
	UStoreItemListEntry* W_Parent;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_OwnedCount;
#pragma endregion
// @@@SNIPEND
};
