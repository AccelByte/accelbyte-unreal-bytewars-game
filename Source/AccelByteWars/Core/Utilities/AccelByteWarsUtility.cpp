// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/Online/HTTP/Public/Http.h"

const TMap<FString, EImageFormat> AccelByteWarsUtility::ImageFormatMap =
{
	{"image/jpeg", EImageFormat::JPEG},
	{"image/png", EImageFormat::PNG},
	{"image/bmp", EImageFormat::BMP}
};

void AccelByteWarsUtility::GetImageFromURL(const FString& Url, const FString& ImageId, const FOnImageReceived& OnReceived)
{
	const FHttpRequestPtr ImageRequest = FHttpModule::Get().CreateRequest();

	ImageRequest->SetVerb("GET");
	ImageRequest->SetURL(Url);

	ImageRequest->OnProcessRequestComplete().BindLambda([OnReceived, ImageId](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			const FString ContentType = Response->GetHeader("Content-Type");

			if (ImageFormatMap.Contains(ContentType))
			{
				const FString ResourceName = FPaths::ProjectSavedDir() / TEXT("Caches") / ImageId;
				TArray<uint8> ImageData = Response->GetContent();
				IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
				FCacheBrush ImageBrush;

				const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormatMap[ContentType]);
				if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()))
				{
					constexpr ERGBFormat RGBFormat = ERGBFormat::BGRA;
					constexpr uint8 BitDepth = 8;

					TArray<uint8> DecodedImage;
					if (ImageWrapper->GetRaw(RGBFormat, BitDepth, DecodedImage))
					{
						if (FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(FName(*ResourceName), ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), DecodedImage))
						{
							ImageBrush = MakeShareable(new FSlateDynamicImageBrush(FName(*ResourceName), FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight())));
							FFileHelper::SaveArrayToFile(ImageData, *ResourceName);
						}
					}
				}

				OnReceived.ExecuteIfBound(ImageBrush);
			}
		}
	});

	ImageRequest->ProcessRequest();
}

FCacheBrush AccelByteWarsUtility::GetImageFromCache(const FString& ImageId)
{
	FCacheBrush ImageBrush;
	if (ImageId.IsEmpty()) 
	{
		return ImageBrush;
	}

	TArray<uint8> ImageData;
	const FString ResourceName = FPaths::ProjectSavedDir() / TEXT("Caches") / ImageId;
	if (FFileHelper::LoadFileToArray(ImageData, *ResourceName))
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

		constexpr EImageFormat ImageFormat = EImageFormat::PNG;
		const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()))
		{
			constexpr ERGBFormat RGBFormat = ERGBFormat::BGRA;
			constexpr uint8 BitDepth = 8;
			TArray<uint8> DecodedImage;

			if (ImageWrapper->GetRaw(RGBFormat, BitDepth, DecodedImage))
			{
				if (FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(FName(*ResourceName), ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), DecodedImage))
				{
					ImageBrush = MakeShareable(new FSlateDynamicImageBrush(FName(*ResourceName), FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight())));
				}
			}
		}
	}

	return ImageBrush;
}
