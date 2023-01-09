// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "GameOverWidget.generated.h"

class UCommonButtonBase;
class UTextBlock;
class UVerticalBox;
class UGameOverLeaderboardEntry;

UCLASS()
class ACCELBYTEWARS_API UGameOverWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_PlayAgain;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Quit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_Winner;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_Leaderboard;

protected:
	UPROPERTY(EditDefaultsOnly, Category = GameOver)
	TSubclassOf<UGameOverLeaderboardEntry> LeaderboardEntryClass;

protected:
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	void PlayGameAgain();
	void QuitGame();

	/** NOTE: Since the InGameGameMode is a Blueprint class, therefore it is impossible to call it's ExitLevel event from C++.
	  * Therefore, this function acts as helper to call InGameGameMode's ExitLevel event from Blueprint.
	  * TODO: Might need to change this after the InGameGameMode is converted to C++. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnExitLevel();

public:
	UFUNCTION(BlueprintCallable)
	void SetWinner(const FText& PlayerName, const FLinearColor& Color);

	UFUNCTION(BlueprintCallable)
	void AddLeaderboardEntry(const FText& PlayerName, const int32 PlayerScore, const int32 PlayerKills, const FLinearColor& PlayerColor);
};
