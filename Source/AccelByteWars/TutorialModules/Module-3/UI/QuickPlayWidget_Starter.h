// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-3/MatchmakingEssentialsSubsystem.h"
#include "TutorialModules/Module-3/MatchmakingEssentialsModels.h"
#include "QuickPlayWidget_Starter.generated.h"

class UAccelByteWarsGameInstance;
class UWidgetSwitcher;
class UCommonButtonBase;

UCLASS()
class ACCELBYTEWARS_API UQuickPlayWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

#pragma region Module.3a Function Declarations

protected:
	// TODO: Add your Module.3a function declarations here.

#pragma endregion


#pragma region Module.3c Function Declarations

protected:
	// TODO: Add your Module.3c function declarations here.

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
	UCommonButtonBase* Btn_Elimination;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_TeamDeathmatch;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Cancel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Ok;
};
