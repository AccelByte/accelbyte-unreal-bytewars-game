// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "LeaderboardsWidget.generated.h"

class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API ULeaderboardsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
// @@@SNIPSTART LeaderboardsWidget.h-public
// @@@MULTISNIP GetLeaderboardGameMode {"selectedLines": ["1-5"]}
public:
	static FString GetLeaderboardGameMode() 
	{
		return LeaderboardGameMode;
	}
// @@@SNIPEND
	
// @@@SNIPSTART LeaderboardsWidget.h-protected
// @@@MULTISNIP LeaderboardUI {"selectedLines": ["1", "9-16"]}
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void OpenLeaderboardsPeriod(const FString InGameMode);

	static FString LeaderboardGameMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_SinglePlayer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Elimination;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_TeamDeathmatch;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> LeaderboardsPeriodWidgetClass;
// @@@SNIPEND
};