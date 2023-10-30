// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "OwnedCountWidgetEntry.generated.h"

class UTextBlock;
class UEntitlementsEssentialsSubsystem;
class UItemDataObject;

UCLASS(Abstract)
class ACCELBYTEWARS_API UOwnedCountWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	UPROPERTY()
	UEntitlementsEssentialsSubsystem* EntitlementsSubsystem;

	void ShowOwnedCount();

#pragma region "UI Related"
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_OwnedCount;
#pragma endregion 
};
