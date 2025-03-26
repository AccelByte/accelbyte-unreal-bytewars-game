// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "MatchmakingP2PWidget.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;
class UWidgetSwitcher;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UMatchmakingP2PWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

// @@@SNIPSTART MatchmakingP2PWidget.h-protected
// @@@MULTISNIP MatchmakingDeclaration {"selectedLines": ["1-12"]}
// @@@MULTISNIP MatchmakingCallbackDeclaration {"selectedLines": ["1", "14-27"]}
protected:
	UFUNCTION()
	void StartMatchmaking();

	UFUNCTION()
	void JoinSession();

	UFUNCTION()
	void CancelMatchmaking();

	UFUNCTION()
	void RejectSessionInvite();

	void OnStartMatchmakingComplete(FName SessionName, bool bSucceeded);
	void OnMatchmakingComplete(FName SessionName, bool bSucceeded);
	void OnSessionInviteReceived(
		const FUniqueNetId& UserId,
		const FUniqueNetId& FromId,
		const FOnlineSessionInviteAccelByte& Invite);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionServerUpdateReceived(
		const FName SessionName,
		const FOnlineError& Error,
		const bool bHasClientTravelTriggered);

	void OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded);
	void OnRejectSessionInviteComplete(bool bSucceeded);
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.h-private
// @@@MULTISNIP OnlineSession {"selectedLines": ["1-3"]}
// @@@MULTISNIP SessionInvite {"selectedLines": ["1", "5"]}
// @@@MULTISNIP SelectedGameModeType {"selectedLines": ["1", "7"]}
private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	TSharedPtr<FOnlineSessionInviteAccelByte> SessionInvite;

	EGameModeType SelectedGameModeType;
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PWidget.h-UI
// @@@MULTISNIP ChangeWidgetState {"selectedLines": ["2", "17"]}
// @@@MULTISNIP StateChanges {"selectedLines": ["19", "30-40"]}
// @@@MULTISNIP LoadingState {"selectedLines": ["19", "42-43", "57-61"]}
// @@@MULTISNIP ErrorState {"selectedLines": ["19", "45-46", "54-55", "63-64"]}
// @@@MULTISNIP WaitingForPlayerState {"selectedLines": ["19", "48-52", "66-67"]}
// @@@MULTISNIP WaitingForPlayerStateCountdown {"selectedLines": ["19", "22", "26"]}
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
// @@@SNIPEND
};
