// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "AccelByteWarsButtonBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetButtonText(const FText& InText);

	UPROPERTY(EditAnywhere, Category = "Tutorial Module Dependency")
	FTutorialModuleDependency TutorialModuleDependency;

protected:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	// End of UUserWidget interface

	// UCommonButtonBase interface
	virtual void UpdateInputActionWidget() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	// End of UCommonButtonBase interface

	void RefreshButtonText();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonText(const FText& InText);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonStyle();
	
private:
	UPROPERTY(EditAnywhere, Category = "Button", meta = (InlineEditConditionToggle))
	uint8 bOverride_ButtonText : 1;

	UPROPERTY(EditAnywhere, Category = "Button", meta = (editcondition = "bOverride_ButtonText"))
	FText ButtonText;
};
