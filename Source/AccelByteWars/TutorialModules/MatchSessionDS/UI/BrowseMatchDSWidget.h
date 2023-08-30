﻿// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "TutorialModules/MatchSessionEssentials/UI/BrowseMatchWidget.h"
#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"
#include "BrowseMatchDSWidget.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBrowseMatchDSWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void CancelJoining() const;
	void FindSessions(const bool bForce) const;
	void JoinSession(const FOnlineSessionSearchResult&SessionSearchResult) const;

	const int32 SessionsNumToQuery = 20;

private:
	void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) const;
	void OnFindSessionComplete(const TArray<FMatchSessionEssentialInfo> SessionEssentialsInfo, bool bSucceeded);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type CompletionType) const;
	void OnSessionServerUpdateReceived(
		const FName SessionName,
		const FOnlineError& Error,
		const bool bHasClientTravelTriggered) const;

	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

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