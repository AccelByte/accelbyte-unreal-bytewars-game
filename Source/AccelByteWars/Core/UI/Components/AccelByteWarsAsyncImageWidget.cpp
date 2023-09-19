// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"

#include "Components/Border.h"
#include "Components/ScaleBox.h"
#include "Components/WidgetSwitcher.h"
#include "Core/Utilities/AccelByteWarsUtility.h"

void UAccelByteWarsAsyncImageWidget::LoadImage(const FString& ImageUrl)
{
	const FString ImageId = FBase64::Encode(ImageUrl);

	Ws_Root->SetActiveWidget(W_Loading);

	// Try to set avatar image from cache.
	const FCacheBrush CacheAvatarBrush = AccelByteWarsUtility::GetImageFromCache(ImageId);
	if (CacheAvatarBrush.IsValid())
	{
		B_Loaded->SetBrush(*CacheAvatarBrush.Get());
		SetImageTint(FLinearColor::White);
		Ws_Root->SetActiveWidget(B_Loaded);
	}
	// Set avatar image from URL if it is not exists in cache.
	else if (!ImageUrl.IsEmpty())
	{
		AccelByteWarsUtility::GetImageFromURL(
			ImageUrl,
			ImageId,
			FOnImageReceived::CreateWeakLambda(this, [this](const FCacheBrush ImageResult)
			{
				if (ImageResult.IsValid())
				{
					B_Loaded->SetBrush(*ImageResult.Get());
					SetImageTint(FLinearColor::White);
					Ws_Root->SetActiveWidget(B_Loaded);
				}
				else
				{
					Ws_Root->SetActiveWidget(B_Default);
				}
			})
		);
	}
	// If no valid avatar, reset it to the default one.
	else 
	{
		Ws_Root->SetActiveWidget(B_Default);
	}
}

void UAccelByteWarsAsyncImageWidget::SetImageTint(const FLinearColor& Color)
{
	B_Default->SetBrushColor(Color);
	B_Loaded->SetBrushColor(Color);
}

void UAccelByteWarsAsyncImageWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	Sb_RootOuter->SetStretch(Stretch);
	Sb_RootOuter->SetStretchDirection(StretchDirection);

	B_Default->SetBrush(DefaultBrush);
	SetImageTint(FLinearColor::White);
}
