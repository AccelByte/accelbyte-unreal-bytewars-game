// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-3/MatchmakingEssentialsSubsystem.h"
#include "QuickPlayWidget.generated.h"

class UAccelByteWarsGameInstance;
class UWidgetSwitcher;
class UCommonButtonBase;

enum class EQuickPlayState
{
	Default,
	FindingMatch,
	JoiningMatch,
	CancelingMatch,
	Failed
};

UCLASS()
class ACCELBYTEWARS_API UQuickPlayWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	void StartMatchmaking(const FString MatchPool);
	void CancelMatchmaking();

	void OnMatchmaking(EMatchmakingState MatchmakingState);
	void SetQuickPlayState(const EQuickPlayState NewState);

private:
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