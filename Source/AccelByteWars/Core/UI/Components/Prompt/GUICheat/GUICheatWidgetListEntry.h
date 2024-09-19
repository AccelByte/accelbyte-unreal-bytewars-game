// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "GUICheatWidgetListEntry.generated.h"

class UListView;
class UGUICheatParamListEntryData;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UGUICheatWidgetListEntry : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	UFUNCTION()
	void OnExecuteClicked(FOnGUICheatWidgetEntryClicked OnClicked) const;

private:
	UPROPERTY()
	TArray<UGUICheatParamListEntryData*> ParamData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Name;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_ParamOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Execute;
};
