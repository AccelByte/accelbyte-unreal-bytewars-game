// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "OwnedCountWidgetEntry_Starter.generated.h"

class UStoreItemListEntry;
class UTextBlock;
class UEntitlementsEssentialsSubsystem_Starter;
class UStoreItemDataObject;

UCLASS(Abstract)
class ACCELBYTEWARS_API UOwnedCountWidgetEntry_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	UPROPERTY()
	UEntitlementsEssentialsSubsystem_Starter* EntitlementsSubsystem;

	UFUNCTION()
	void CheckItemEquipped();

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion 

#pragma region "UI"
private:
	UPROPERTY()
	UStoreItemListEntry* W_Parent;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_OwnedCount;
#pragma endregion 
};
