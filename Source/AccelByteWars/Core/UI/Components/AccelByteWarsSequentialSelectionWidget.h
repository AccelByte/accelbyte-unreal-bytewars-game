// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Core/UI/AccelByteWarsWidgetInterface.h"
#include "AccelByteWarsSequentialSelectionWidget.generated.h"

class UTextBlock;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsSequentialSelectionWidget : public UCommonUserWidget, public IAccelByteWarsWidgetInterface
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FNavigationReply NativeOnNavigation(
		const FGeometry& MyGeometry,
		const FNavigationEvent& InNavigationEvent,
		const FNavigationReply& InDefaultReply) override;

public:
	void SetSelection(TArray<FText>& InSelections, int32 DefaultIndex = 0);
	void ClearSelection();

	void SetDisplayName(const FText InText) const;

	void SetSelectedIndex(const int32 Index);
	int32 GetSelectedIndex() const { return CurrentIndex; }
	FText GetSelected() const { return CurrentIndex > INDEX_NONE ? Selections[CurrentIndex] : FText(); }

private:
	enum class ECycleDirection : uint8
	{
		RIGHT = 0,
		LEFT = 1
	};

	void CycleSelection(const ECycleDirection Direction);
	void DrawSelection();

	UPROPERTY(EditInstanceOnly)
	TArray<FText> Selections;

	UPROPERTY(EditInstanceOnly)
	int32 CurrentIndex = 0;

	UPROPERTY(EditInstanceOnly)
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Selected;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_CycleRight;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_CycleLeft;
};
