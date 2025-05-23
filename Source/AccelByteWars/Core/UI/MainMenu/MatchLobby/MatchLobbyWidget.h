// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"
#include "MatchLobbyWidget.generated.h"

class UAccelByteWarsGameInstance;
class AAccelByteWarsMainMenuGameState;
class UAccelByteWarsWidgetSwitcher;
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

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnQueryTeamMembersInfo,
	const APlayerController* /*PlayerController*/,
	const TArray<FUniqueNetIdRef>& /*MemberUserIds*/, 
	const TDelegate<void(const TMap<FUniqueNetIdRepl, FGameplayPlayerData>& /*TeamMembersInfo*/)> /*OnComplete*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuitLobby, APlayerController* /*PlayerController*/);

UCLASS()
class ACCELBYTEWARS_API UMatchLobbyWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void StartMatch();

	UFUNCTION(BlueprintCallable)
	void LeaveMatch();

	inline static FOnQueryTeamMembersInfo OnQueryTeamMembersInfoDelegate;
	inline static FOnQuitLobby OnQuitLobbyDelegate;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

// @@@SNIPSTART MatchLobbyWidget.h-private
// @@@MULTISNIP MatchActionUI {"selectedLines": ["1", "20-24"]}
private:
	UFUNCTION()
	void ShowLoading();

	void QueryTeamMembersInfo();
	void GenerateMultiplayerTeamEntries(const TMap<FUniqueNetIdRepl, FGameplayPlayerData>& AdditionalMembersInfo = {});

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
	UAccelByteWarsWidgetSwitcher* Ws_TeamList;

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
// @@@SNIPEND
};