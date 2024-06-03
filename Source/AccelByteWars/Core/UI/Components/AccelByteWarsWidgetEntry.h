// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "AccelByteWarsWidgetEntry.generated.h"

class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsWidgetEntry : public UAccelByteWarsActivatableWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	FSimpleMulticastDelegate* GetOnListItemObjectSet()
	{
		return &OnListItemObjectSet;
	}

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
	virtual void NativeConstruct() override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	UFUNCTION(BlueprintImplementableEvent)
	TArray<UCommonButtonBase*> InputMethodDependantWidgets();

	FSimpleMulticastDelegate OnListItemObjectSet;

private:
	UFUNCTION()
	void ChangeInteractibility(ECommonInputType InputType);

	UPROPERTY()
	UCommonInputSubsystem* InputSubsystem;

	bool bIsItemSelected = false;
};