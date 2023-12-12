// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "MatchmakingP2PWidget_Starter.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UCommonButtonBase;
class UQuickPlayWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UMatchmakingP2PWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

#pragma region "Matchmaking with P2P Function Declarations"
protected:
	// TODO: Add your module function definitions here.
#pragma endregion 

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

#pragma region "UI related"
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_StartMatchmakingP2P;

	UPROPERTY()
	UQuickPlayWidget* W_Parent;
#pragma endregion 
};
