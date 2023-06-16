// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/ListView.h"
#include "AccelByteWarsWidgetList.generated.h"

UENUM(BlueprintType)
enum class EAccelByteWarsWidgetListType : uint8
{
	ListView = 0,
	TileView
};

UENUM(BlueprintType)
enum class EAccelByteWarsWidgetListState : uint8
{
	EntryLoaded = 0,
	LoadingEntry,
	NoEntry,
	Error
};

class UWidgetSwitcher;
class UAccelByteWarsListView;
class UAccelByteWarsTileView;
class UAccelByteWarsWidgetEntry;
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
	UListView* GetListView() const { return SelectedListViewType; };

protected:
	void NativePreConstruct() override;
	void NativeConstruct() override;

	UListView* SelectedListViewType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAccelByteWarsWidgetListType ListViewType;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAccelByteWarsWidgetEntry> EntryWidgetClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "ListViewType==EAccelByteWarsWidgetListType::TileView", EditConditionHides))
	float EntryWidgetWidth;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "ListViewType==EAccelByteWarsWidgetListType::TileView", EditConditionHides))
	float EntryWidgetHeight;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ListViewType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsListView* ListView;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsTileView* TileView;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ListViewState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_NoEntryMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FailedMessage;
};