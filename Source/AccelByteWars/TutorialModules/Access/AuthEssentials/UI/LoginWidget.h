// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/AuthEssentials/AuthEssentialsSubsystem.h"
#include "LoginWidget.generated.h"

UENUM(BlueprintType)
enum ELoginState 
{
	Default,
	Failed
};

UCLASS()
class ACCELBYTEWARS_API ULoginWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
			
protected:
	UFUNCTION(BlueprintCallable)
	void Login(EAccelByteLoginType LoginMethod, const FAuthOnLoginComplete& OnLoginComplete);

	UPROPERTY(BlueprintReadOnly)
	EAccelByteLoginType LastLoginMethod;
};