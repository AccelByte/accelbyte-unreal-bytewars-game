// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Play/MatchSessionEssentials/UI/BrowseMatchWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"
#include "BrowseMatchP2PWidget_Starter.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBrowseMatchP2PWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

#pragma region "Match Session with P2P function declarations"
protected:
	// TODO: Add your function declarations here
#pragma endregion

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
