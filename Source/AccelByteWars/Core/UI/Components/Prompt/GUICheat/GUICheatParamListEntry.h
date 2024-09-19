// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "GUICheatParamListEntry.generated.h"

class UTextBlock;
class UEditableText;

UCLASS()
class ACCELBYTEWARS_API UGUICheatParamListEntryData : public UObject
{
	GENERATED_BODY()

public:
	FText ParamName;
};

UCLASS(Abstract)
class ACCELBYTEWARS_API UGUICheatParamListEntry : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:
	FString GetParamValue() const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_ParamName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Et_ParamValue;
};
