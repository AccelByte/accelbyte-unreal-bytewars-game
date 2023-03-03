// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "MatchLobbyWidget.generated.h"

class UUtilitiesSubsystem;
class UAccelByteWarsGameInstance;
class AAccelByteWarsGameStateBase;
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

DECLARE_DELEGATE_TwoParams(FOnGenerateOnlineTeamEntries, TArray<FUniqueNetIdPtr> /*PlayerUniqueNetIds*/, TArray<UPlayerEntryWidget*> /*PlayerEntryWidgets*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuitLobby, APlayerController* /*PlayerController*/);
typedef FOnQuitLobby::FDelegate FOnQuitLobbyDelegate;

UCLASS()
class ACCELBYTEWARS_API UMatchLobbyWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void GenerateMultiplayerTeamEntries(const bool bIsOnline = false);

	UFUNCTION(BlueprintCallable)
	void StartMatch();

	UFUNCTION(BlueprintCallable)
	void LeaveMatch();

	inline static FOnGenerateOnlineTeamEntries OnGenerateOnlineTeamEntries;
	inline static FOnQuitLobbyDelegate OnQuitLobbyDelegate;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
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
	AAccelByteWarsGameStateBase* GameState;
	TArray<UPlayerEntryWidget*> PlayerEntries;
};