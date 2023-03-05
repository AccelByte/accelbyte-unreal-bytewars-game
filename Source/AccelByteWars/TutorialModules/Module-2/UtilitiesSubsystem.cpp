// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-2/UtilitiesSubsystem.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteRegistry.h"

TMap<FString, EImageFormat> UUtilitiesSubsystem::ImageFormatMap =
{
	{"image/jpeg", EImageFormat::JPEG},
	{"image/png", EImageFormat::PNG},
	{"image/bmp", EImageFormat::BMP}
};

bool UUtilitiesSubsystem::IsAccelByteSDKInitialized()
{
    bool IsOSSEnabled = true;
    bool IsSDKCredsEmpty = false;

    // Check AccelByte Subsystem.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem) || !Subsystem->IsEnabled())
    {
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("AccelByte SDK and OSS is not valid."));
        IsOSSEnabled = false;
    }

    if (IsRunningDedicatedServer()) 
    {
        // Check server credentials.
        ServerSettings ServerCreds = FRegistry::ServerSettings;
        if (ServerCreds.ClientId.IsEmpty() || ServerCreds.ClientSecret.IsEmpty() ||
            ServerCreds.Namespace.IsEmpty() || ServerCreds.PublisherNamespace.IsEmpty() || ServerCreds.BaseUrl.IsEmpty())
        {
            UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Server creds are empty or not filled properly. Please check your AccelByte SDK settings configuration."));
            IsSDKCredsEmpty = true;
        }
    }
    else 
    {
        // Check client credentials.
        Settings ClientCreds = FRegistry::Settings;
        if (ClientCreds.ClientId.IsEmpty() || ClientCreds.Namespace.IsEmpty() ||
            ClientCreds.PublisherNamespace.IsEmpty() || ClientCreds.BaseUrl.IsEmpty())
        {
            UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Client creds are empty or not filled properly. Please check your AccelByte SDK settings configuration."));
            IsSDKCredsEmpty = true;
        }
    }

    return IsOSSEnabled && !IsSDKCredsEmpty;
}

void UUtilitiesSubsystem::GetImageFromURL(const FString& Url, const FString& ImageId, const FOnImageReceived& OnReceived)
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

FCacheBrush UUtilitiesSubsystem::GetImageFromCache(const FString& ImageId)
{
	TArray<uint8> ImageData;
	FCacheBrush ImageBrush;
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