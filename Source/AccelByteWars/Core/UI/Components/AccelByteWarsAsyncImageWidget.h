// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Widgets/Layout/SScaleBox.h"
#include "AccelByteWarsAsyncImageWidget.generated.h"

class UWidgetSwitcher;
class UBorder;
class UScaleBox;

/**
 * Image widget with default, loading, and loaded state. Call 
 */
UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsAsyncImageWidget final : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Retrieve image from the given URL. Will use DefaultBrush if URL isn't valid
	 * @param ImageUrl URL to retrieve image from
	 */
	UFUNCTION(BlueprintCallable)
	void LoadImage(const FString& ImageUrl);

	void SetImageTint(const FLinearColor& Color);

	/** The stretching rule to apply when content is stretched */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EStretch::Type> Stretch;

	/** Controls in what direction content can be scaled */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EStretchDirection::Type> StretchDirection;

	UPROPERTY(EditAnywhere)
	FSlateBrush DefaultBrush;

protected:
	virtual void NativePreConstruct() override;

private:
#pragma region "UI components"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UScaleBox* Sb_RootOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UBorder* B_Default;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UBorder* B_Loaded;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Loading;
#pragma endregion 
};
