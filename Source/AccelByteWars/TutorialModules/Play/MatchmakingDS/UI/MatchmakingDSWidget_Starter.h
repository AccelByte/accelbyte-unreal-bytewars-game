// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MatchmakingDSWidget_Starter.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;
class UWidgetSwitcher;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UMatchmakingDSWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

#pragma region "Matchmaking with DS Function Declarations"
protected:
	// TODO: Add your module function definitions here.
#pragma endregion 

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	TSharedPtr<FOnlineSessionInviteAccelByte> SessionInvite;

	EGameModeType SelectedGameModeType;

#pragma region "UI related"
protected:
	enum class EWidgetState : uint8
	{
		REQUEST_SENT,
		FINDING_MATCH,
		MATCH_FOUND,
		CANCELING_MATCH,
		WAITING_FOR_PLAYER,
		REJECTING_MATCH,
		JOINING_MATCH,
		SESSION_JOINED,
		REQUESTING_SERVER,
		ERROR
	};

	void ChangeWidgetState(const EWidgetState State);

private:
	EWidgetState WidgetState = EWidgetState::REQUEST_SENT;

	const float AutoJoinDelay = 10;
	const float MatchFoundDelay = 1;
	const float SessionJoinedDelay = 1;

	float AutoJoinCurrentCountdown = 0;
	float MatchFoundCurrentCountdown = 0;
	float SessionJoinedCurrentCountdown = 0;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Root;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Loading;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Error;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_WaitingForPlayer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Cancel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Reject;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Join;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Retry;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_LoadingText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_LoadingSubText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_ErrorText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_WaitingForPlayersCountdown;
#pragma endregion  
};
