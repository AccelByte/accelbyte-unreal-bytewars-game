// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-3/MatchmakingEssentialsModels.h"
#include "QuickPlayWidget.generated.h"

class UAccelByteWarsGameInstance;
class UWidgetSwitcher;
class UTextBlock;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UQuickPlayWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

#pragma region Module.3c Function Declarations

public:
	void OnMatchmaking(EMatchmakingState MatchmakingState, FString ErrorMessage);

#pragma endregion

public:
	FString GetMatchGameMode() const;

	FOnRequestCancelMatchmaking OnRequestCancelMatchmaking;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	void SetQuickPlayState(const EMatchmakingState NewState);
	void SetMatchGameMode(const FString& InMatchGameMode);

private:
	void OnEliminationButtonClicked();
	void OnTeamDeathmatchButtonClicked();

	void OnDedicatedServerTypeButtonClicked();
	void OnCancelMatchmakingButtonClicked();

	UAccelByteWarsGameInstance* GameInstance;
	FString MatchPoolGameMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_QuickPlayState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_QuickPlayMenu;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FailedMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Elimination;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_TeamDeathmatch;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_DedicatedServerType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Cancel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Ok;
};