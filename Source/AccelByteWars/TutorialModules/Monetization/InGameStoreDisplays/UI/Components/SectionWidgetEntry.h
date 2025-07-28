// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "SectionWidgetEntry.generated.h"

class UAccelByteWarsActivatableWidget;
class UTextBlock;
class UBorder;
class UListView;

UCLASS(Abstract)
class ACCELBYTEWARS_API USectionWidgetEntry : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()

// @@@SNIPSTART SectionWidgetEntry.h
// @@@MULTISNIP NativeOnListItemObjectSet {"selectedLines": ["1"]}
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
// @@@SNIPEND

// @@@SNIPSTART SectionWidgetEntry.h-protected
protected:
	void OnItemClicked(UObject* Item) const;
// @@@SNIPEND

// @@@SNIPSTART SectionWidgetEntry.h-private
// @@@MULTISNIP UI {"selectedLines": ["1", "4-17"]}
// @@@MULTISNIP TimeLeft {"selectedLines": ["1-2"]}
private:
	FTimespan TimeLeft;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> DetailWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_TimeLeft;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_SectionName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UBorder* B_SectionBorder;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Items;
// @@@SNIPEND
};
