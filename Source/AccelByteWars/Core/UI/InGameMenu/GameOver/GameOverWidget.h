// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"
#include "GameOverWidget.generated.h"

class AAccelByteWarsInGameGameState;
class UCommonButtonBase;
class UTextBlock;
class UVerticalBox;
class UWidgetSwitcher;
class UGameOverLeaderboardEntry;
class UAccelByteWarsGameInstance;
class AAccelByteWarsGameState;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuitGame, APlayerController* /*PlayerController*/);

UCLASS()
class ACCELBYTEWARS_API UGameOverWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	UGameOverLeaderboardEntry* AddLeaderboardEntry(const FText& PlayerName, const int32 PlayerScore, const int32 PlayerKills, const FLinearColor& PlayerColor);

	inline static FOnQuitGame OnQuitGameDelegate;

protected:
	//~UUserWidget overriden functions
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	//~End of UUserWidget overriden function

	/** NOTE: Since the InGameGameMode is a Blueprint class, therefore it is impossible to call it's ExitLevel event from C++.
	  * Therefore, this function acts as helper to call InGameGameMode's ExitLevel event from Blueprint.
	  * TODO: Might need to change this after the InGameGameMode is converted to C++. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnExitLevel();

private:
	void SetupLeaderboard();
	void PlayGameAgain();
	void QuitGame();

	UPROPERTY(EditDefaultsOnly, Category = GameOver)
	TSubclassOf<UGameOverLeaderboardEntry> LeaderboardEntryClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_PlayAgain;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Quit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_Winner;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_Leaderboard;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* Widget_Countdown;

	UFUNCTION()
	ECountdownState GetCountdownState();

	UFUNCTION()
	int GetCountdownValue();

	UFUNCTION()
	void OnCountdownFinished();

	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance;
	AAccelByteWarsInGameGameState* GameState;
};