// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Play/MatchSessionEssentials/UI/BrowseMatchWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"
#include "BrowseMatchDSWidget.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBrowseMatchDSWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	void FindSessions(const bool bForce) const;
	void OnFindSessionComplete(const TArray<FMatchSessionEssentialInfo> SessionEssentialsInfo, bool bSucceeded);

	void CancelJoining() const;
	void OnCancelJoiningComplete(FName SessionName, bool bSucceeded) const;

	void JoinSession(const FOnlineSessionSearchResult&SessionSearchResult) const;
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type CompletionType) const;

	void OnSessionServerUpdateReceived(
		const FName SessionName,
		const FOnlineError& Error,
		const bool bHasClientTravelTriggered) const;

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	const int32 SessionsNumToQuery = 20;

#pragma region "UI related"
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Refresh;

	UPROPERTY()
	UBrowseMatchWidget* W_Parent;
#pragma endregion 
};
