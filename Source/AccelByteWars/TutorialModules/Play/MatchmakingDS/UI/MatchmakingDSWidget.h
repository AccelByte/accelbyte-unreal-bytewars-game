// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MatchmakingDSWidget.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;
class UQuickPlayWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UMatchmakingDSWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	UFUNCTION()
	void StartMatchmaking() const;

	UFUNCTION()
	void CancelButtonClicked() const;

	void OnStartMatchmakingComplete(FName SessionName, bool bSucceeded) const;
	void OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded) const;
	void OnMatchmakingComplete(FName SessionName, bool bSucceeded) const;
	void OnCancelJoinSessionComplete(FName SessionName, bool bSucceeded) const;
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) const;

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

#pragma region "UI related"
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_StartMatchmakingDS;

	UPROPERTY()
	UQuickPlayWidget* W_Parent;
#pragma endregion 
};
