// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/PresenceEssentials/PresenceEssentialsSubsystem_Starter.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PresenceWidget_Starter.generated.h"

class UCommonTextBlock;
class UThrobber;
class UListViewBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPresenceWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void SetupPresence();
	void OnSetupPresenceComplete();

#pragma region Module Presence Essentials Function Declarations
	// TODO: Add your protected function declarations here.
#pragma endregion

	UPROPERTY()
	UPresenceEssentialsSubsystem_Starter* PresenceEssentialsSubsystem;

	FUniqueNetIdPtr PresenceUserId;

	UPROPERTY()
	UListViewBase* ParentListView;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Tb_Presence;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UThrobber* Th_Loader;
};
