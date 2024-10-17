// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "Social/RecentPlayers/RecentPlayersSubsystem.h"
#include "RecentPlayersWidget.generated.h"

class UAccelByteWarsGameInstance;
class UAccelByteWarsWidgetSwitcher;
class UTileView;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API URecentPlayersWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
// @@@SNIPSTART RecentPlayersWidget.h-protected
// @@@MULTISNIP RecentPlayersUI {"selectedLines": ["1", "19-23"]}
// @@@MULTISNIP InitRecentPlayerDeclaration {"selectedLines": ["1", "8-9", "17"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitRecentPlayersList();
	void OnRecentPlayerEntryClicked(UObject* Item);

	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance;

	UPROPERTY()
	URecentPlayersSubsystem* RecentPlayersSubsystem;

	FDelegateHandle OnRecentPlayersListUpdatedDelegateHandle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_RecentPlayers;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_RecentPlayers;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> RecentPlayerDetailsWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersWidget.h-private
// @@@MULTISNIP OnQueryRecentPlayerComplete {"selectedLines": ["1-3"]}
private:
	void OnQueryRecentPlayerComplete(const FUniqueNetId& UserId, const FString& Namespace, bool bWasSuccessful, const FString& Error);
	FOnQueryRecentPlayersCompleteDelegate OnQueryRecentPlayersCompletedDelegate;
// @@@SNIPEND
};