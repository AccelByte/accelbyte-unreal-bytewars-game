// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsUtility.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
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
		else
		{
			OnReceived.ExecuteIfBound(FCacheBrush());
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

int32 AccelByteWarsUtility::PositiveModulo(const int32 Dividend, const int32 Modulus)
{
	return Modulus == 0 ? INDEX_NONE : (Modulus + (Dividend % Modulus)) % Modulus;
}

FString AccelByteWarsUtility::GetGameVersion()
{
	const FString ProjectVerSectionPath = FString("/Script/EngineSettings.GeneralProjectSettings");
	const FString ProjectVerConfig = FString("ProjectVersion");

	FString ProjectVersionStr = FString();
	GConfig->GetString(*ProjectVerSectionPath, *ProjectVerConfig, ProjectVersionStr, GGameIni);

	return ProjectVersionStr;
}

TArray<UUserWidget*> AccelByteWarsUtility::FindWidgetsOnTheScreen(
	const FString& WidgetName, 
	const TSubclassOf<UUserWidget> WidgetClass, 
	const bool bTopLevelOnly, 
	UObject* Context)
{
	TArray<UUserWidget*> FoundWidgets;
	if (WidgetName.IsEmpty() || !WidgetClass.Get()) 
	{
		return FoundWidgets;
	}

	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(Context, FoundWidgets, WidgetClass.Get(), bTopLevelOnly);
	FoundWidgets.RemoveAll([WidgetName](const UUserWidget* FoundWidget)
	{
		return !FoundWidget || !FoundWidget->GetName().Equals(WidgetName) || !FoundWidget->IsVisible();
	});

	return FoundWidgets;
}

bool AccelByteWarsUtility::IsUseVersionChecker()
{
	// Check for launch parameter first.
	const FString CmdArgs = FCommandLine::Get();
	const FString CmdStr = FString("-UseVersionChecker=");
	bool bValidCmdValue = false;
	bool bUseVersionChecker = false;
	if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
	{
		FString CmdValue;
		FParse::Value(*CmdArgs, *CmdStr, CmdValue);
		if (!CmdValue.IsEmpty())
		{
			bUseVersionChecker = CmdValue.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			bValidCmdValue = true;
		}
	}

	// Check for DefaultEngine.ini
	if (!bValidCmdValue)
	{
		GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("bUseVersionChecker"), bUseVersionChecker, GEngineIni);
	}

	return bUseVersionChecker;
}
