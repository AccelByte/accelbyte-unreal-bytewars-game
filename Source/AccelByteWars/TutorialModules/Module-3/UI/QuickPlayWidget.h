// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-3/MatchmakingEssentialsSubsystem.h"
#include "TutorialModules/Module-3/MatchmakingEssentialsModels.h"
#include "QuickPlayWidget.generated.h"

class UAccelByteWarsGameInstance;
class UWidgetSwitcher;
class UTextBlock;
class UCommonButtonBase;

UCLASS()
class ACCELBYTEWARS_API UQuickPlayWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

#pragma region Module.3a Function Declarations
protected:
	void StartMatchmaking(const FString MatchPool);
	void CancelMatchmaking();
#pragma endregion

#pragma region Module.3c Function Declarations

protected:
	void OnMatchmaking(EMatchmakingState MatchmakingState, FString ErrorMessage);

#pragma endregion

protected:
	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void SetQuickPlayState(const EMatchmakingState NewState);

private:
	void OnEliminationButtonClicked();
	void OnTeamDeathmatchButtonClicked();
	void OnCancelMatchmakingClicked();

	UAccelByteWarsGameInstance* GameInstance;
	UMatchmakingEssentialsSubsystem* MatchmakingSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_QuickPlayState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FailedMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Elimination;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_TeamDeathmatch;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Cancel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Ok;
};