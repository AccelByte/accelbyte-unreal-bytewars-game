// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "Social/RecentPlayers/RecentPlayersSubsystem.h"
#include "PlayersListWidget.generated.h"

class UAccelByteWarsGameInstance;
class UAccelByteWarsWidgetSwitcher;
class UTileView;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPlayersListWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
// @@@SNIPSTART PlayersListWidget.h-protected
// @@@MULTISNIP PlayersListUI {"selectedLines": ["1", "21-25"]}
// @@@MULTISNIP InitGameSessionPlayerListDeclaration {"selectedLines": ["1", "7-13"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void InitGameSessionPlayersList();
	void OnPlayersListEntryClicked(UObject* Item);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	void OnSessionParticipantsJoined(FName SessionName, const FUniqueNetId& User);
#else
	void OnSessionParticipantsChanged(FName SessionName, const FUniqueNetId& User, bool bJoined);
#endif

	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance;
	
	UPROPERTY()
	URecentPlayersSubsystem* RecentPlayersSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_PlayersList;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_PlayersList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> PlayersListDetailsWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
// @@@SNIPEND
};
