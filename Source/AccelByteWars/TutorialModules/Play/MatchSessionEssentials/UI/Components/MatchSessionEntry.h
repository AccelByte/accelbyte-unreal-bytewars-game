// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"
#include "MatchSessionEntry.generated.h"

class UTextBlock;
class UCommonButtonBase;
class UAccelByteWarsAsyncImageWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UMatchSessionEntry : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;

private:
	void UiSetup() const;

	FMatchSessionEssentialInfo SessionEssentialInfo;
	TDelegate<void(const FOnlineSessionSearchResult&)> OnJoinButtonClicked;

#pragma region "UI components"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsAsyncImageWidget* W_OwnerAvatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_OwnerDisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_GameModeType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_NetworkType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_RegisteredPlayerCount;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Join;
#pragma endregion 
};
