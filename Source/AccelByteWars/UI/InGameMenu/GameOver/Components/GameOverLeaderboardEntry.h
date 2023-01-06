// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UI/AccelByteWarsActivatableWidget.h"
#include "GameOverLeaderboardEntry.generated.h"

class UTextBlock;

UCLASS()
class ACCELBYTEWARS_API UGameOverLeaderboardEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_PlayerName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_PlayerKills;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_PlayerScore;

public:
	UFUNCTION(BlueprintCallable)
	void InitData(const FText& PlayerName, const int32 PlayerScore, const int32 PlayerKills, const FLinearColor& PlayerColor);
};