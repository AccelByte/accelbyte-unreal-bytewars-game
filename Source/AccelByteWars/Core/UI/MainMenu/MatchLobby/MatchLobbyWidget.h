// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"
#include "MatchLobbyWidget.generated.h"

class UAccelByteWarsGameInstance;
class AAccelByteWarsMainMenuGameState;
class UPanelWidget;
class UCommonButtonBase;
class UWidgetSwitcher;
class UTeamEntryWidget;
class UPlayerEntryWidget;
class UCountdownWidget;

enum class EMatchLobbyState
{
	Default,
	GameStarted
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnterLobby, APlayerController* /*PlayerController*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuitLobby, APlayerController* /*PlayerController*/);

UCLASS()
class ACCELBYTEWARS_API UMatchLobbyWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void GenerateMultiplayerTeamEntries();

	UFUNCTION(BlueprintCallable)
	void StartMatch();

	UFUNCTION(BlueprintCallable)
	void LeaveMatch();

	inline static FOnEnterLobby OnEnterLobbyDelegate;
	inline static FOnQuitLobby OnQuitLobbyDelegate;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void ShowLoading();

	void SetMatchLobbyState(const EMatchLobbyState NewState);
	void ResetTeamEntries();

	UFUNCTION()
	ECountdownState SetCountdownState();
	
	UFUNCTION()
	int UpdateCountdownValue();
	
	UFUNCTION()
	void OnCountdownFinished();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Start;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Quit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_MatchLobby;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* Panel_TeamList;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* CountdownWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTeamEntryWidget> TeamEntryWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPlayerEntryWidget> PlayerEntryWidget;

	UAccelByteWarsGameInstance* GameInstance;
	AAccelByteWarsMainMenuGameState* GameState;
};