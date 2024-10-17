// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Play/MatchSessionEssentials/UI/BrowseMatchWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"
#include "BrowseMatchP2PWidget.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBrowseMatchP2PWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

// @@@SNIPSTART BrowseMatchP2PWidget.h-protected
// @@@MULTISNIP FindSessionDeclaration {"selectedLines": ["1-3"]}
// @@@MULTISNIP CancelJoinSessionDeclaration {"selectedLines": ["1", "5-6"]}
// @@@MULTISNIP JoinSessionDeclaration {"selectedLines": ["1", "8-9"]}
// @@@MULTISNIP OnSessionServerUpdateReceived {"selectedLines": ["1", "11-14"]}
// @@@MULTISNIP OnlineSession {"selectedLines": ["1", "16-17"]}
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

	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	const int32 SessionsNumToQuery = 20;
// @@@SNIPEND

#pragma region "UI related"
// @@@SNIPSTART BrowseMatchP2PWidget.h-private
// @@@MULTISNIP BrowseMatchUI {"selectedLines": ["1-6"]}
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Refresh;

	UPROPERTY()
	UBrowseMatchWidget* W_Parent;
// @@@SNIPEND
#pragma endregion 
};
