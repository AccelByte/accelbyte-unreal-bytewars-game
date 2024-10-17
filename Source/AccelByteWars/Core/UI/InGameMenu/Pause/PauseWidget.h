// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PauseWidget.generated.h"

class UCommonButtonBase;
class UVerticalBox;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuitGame, APlayerController* /*PlayerController*/);

UCLASS()
class ACCELBYTEWARS_API UPauseWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	inline static FOnQuitGame OnQuitGameDelegate;

protected:
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	/** NOTE: Since the InGameGameMode is a Blueprint class, therefore it is impossible to call it's ExitLevel event from C++.
	  * Therefore, this function acts as helper to call InGameGameMode's ExitLevel event from Blueprint.
	  * TODO: Might need to change this after the InGameGameMode is converted to C++. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnExitLevel();

// @@@SNIPSTART PauseWidget.h-private
// @@@MULTISNIP PauseActionUI {"selectedLines": ["1", "7-17"]}
private:
	void ResumeGame();
	void RestartGame();
	void OpenHelpOptions();
	void QuitGame();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Resume;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Restart;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_HelpOptions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Quit;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> HelpOptionsWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_OnlinePlayButtons;
// @@@SNIPEND
};