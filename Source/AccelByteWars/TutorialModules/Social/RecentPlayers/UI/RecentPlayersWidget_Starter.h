// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Social/RecentPlayers/RecentPlayersSubsystem_Starter.h"
#include "RecentPlayersWidget_Starter.generated.h"

class UAccelByteWarsGameInstance;
class UAccelByteWarsWidgetSwitcher;
class UTileView;
class UCommonButtonBase;

UCLASS(Abstract)

class ACCELBYTEWARS_API URecentPlayersWidget_Starter : public UAccelByteWarsActivatableWidget 
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
#pragma region "Tutorial"
	// put your code here
#pragma endregion
	
	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance;

	UPROPERTY()
	URecentPlayersSubsystem_Starter* RecentPlayersSubsystem;

#pragma region "UI"	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_RecentPlayers;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_RecentPlayers;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> RecentPlayerDetailsWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
#pragma endregion

#pragma region Module Recent Players Declarations
	// TODO: Add your Module Recent Players code here.
#pragma endregion
};
