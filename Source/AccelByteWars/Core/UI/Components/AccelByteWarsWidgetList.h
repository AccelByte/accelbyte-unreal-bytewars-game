// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/ListView.h"
#include "AccelByteWarsWidgetList.generated.h"

UENUM(BlueprintType)
enum class EAccelByteWarsWidgetListState : uint8
{
	EntryLoaded = 0,
	LoadingEntry,
	NoEntry,
	Error
};

class UWidgetSwitcher;
class UNamedSlot;
class UListView;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsWidgetList : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void ChangeWidgetListState(const EAccelByteWarsWidgetListState WidgetListState);

	UFUNCTION(BlueprintCallable)
	void SetNoEntryMessage(const FText& Message);

	UFUNCTION(BlueprintCallable)
	void SetFailedMessage(const FText& Message);

	UFUNCTION(BlueprintPure)
	UListView* GetListView() const { return ListView; };

protected:
	void NativeConstruct() override;

	UPROPERTY();
	UListView* ListView;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ListViewState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UNamedSlot* ListViewSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_NoEntryMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FailedMessage;
};