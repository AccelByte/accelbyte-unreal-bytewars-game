// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Monetization/InGameStoreDisplays/InGameStoreDisplaysSubsystem_Starter.h"
#include "SectionedShopWidget_Starter.generated.h"

class UListView;
class UAccelByteWarsWidgetSwitcher;

UCLASS(Abstract)
class ACCELBYTEWARS_API USectionedShopWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	UPROPERTY()
	UInGameStoreDisplaysSubsystem_Starter* InGameStoreDisplaysSubsystem;

	UPROPERTY()
	TArray<USectionDataObject*> SectionDatas;
	
	UPROPERTY(EditAnywhere)
	FString TargetDisplayName;

#pragma region "Tutorial"
	// Put your code here
#pragma endregion 

#pragma region "UI"
private:
	FLinearColor GetSectionPresetColor(const int Index) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Root;

	const TArray<FLinearColor> SectionBackgroundColorPreset = {
		FLinearColor(0.27f, 0.004f, 0.3f),
		FLinearColor(0.2f, 0.25f, 0.3f),
		FLinearColor(0.32f, 0.24f, 0.23f)
	};
#pragma endregion 
};
