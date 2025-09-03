// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "CustomMatchWidget_Starter.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UAccelByteWarsSequentialSelectionWidget;
class UCommonButtonBase;
class UCustomMatchSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCustomMatchWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	UPROPERTY()
	UCustomMatchSubsystem* Subsystem;

#pragma region "Tutorial"
	// Insert your codes here
#pragma endregion 

#pragma region "UI related"
private:
	enum class EContentType : uint8
	{
		CREATE = 0,
		LOADING,
		ERROR
	};
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void OnGameModeTypeSelectionChanged(int32 Index) const;
	void SwitchContent(const EContentType State);
	void ReturnToCreateMenu();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_GameModeTypeSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_GameStyleSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_NetworkTypeSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_JoinabilitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_DurationSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_PlayerLivesSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_MissileLimitSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_MaxTotalPlayerSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* W_MaxTeamSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Create;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back_Error;
#pragma endregion 
};
