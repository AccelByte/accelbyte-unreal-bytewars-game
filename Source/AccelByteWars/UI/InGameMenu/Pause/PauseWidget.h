// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UI/AccelByteWarsActivatableWidget.h"
#include "PauseWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class ACCELBYTEWARS_API UPauseWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Resume;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Restart;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Quit;

protected:
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	UFUNCTION(BlueprintCallable)
	void ResumeGame();

	UFUNCTION(BlueprintCallable)
	void RestartGame();

	UFUNCTION(BlueprintCallable)
	void QuitGame();

	/** NOTE: Since the InGameGameMode is a Blueprint class, therefore it is impossible to call it's ExitLevel event from C++. 
	  * Therefore, this function acts as helper to call InGameGameMode's ExitLevel event from Blueprint. 
	  * TODO: Might need to change this after the InGameGameMode is converted to C++. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnExitLevel();
};