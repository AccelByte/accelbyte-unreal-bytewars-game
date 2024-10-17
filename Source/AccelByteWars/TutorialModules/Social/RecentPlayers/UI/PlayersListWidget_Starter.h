// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "Social/RecentPlayers/RecentPlayersSubsystem_Starter.h"
#include "PlayersListWidget_Starter.generated.h"

class UAccelByteWarsGameInstance;
class UAccelByteWarsWidgetSwitcher;
class UTileView;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPlayersListWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

#pragma region "UI"
	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance;
	
	UPROPERTY()
	URecentPlayersSubsystem_Starter* RecentPlayersSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_PlayersList;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_PlayersList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> PlayersListDetailsWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
#pragma endregion

#pragma region Module Recent Players Declarations
	// TODO: Add your Module Recent Players code here.
#pragma endregion
};
