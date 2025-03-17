// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsUtility.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Engine/Classes/GameFramework/PlayerState.h"
#include "EngineSettings/Classes/GeneralProjectSettings.h"

const TMap<FString, EImageFormat> AccelByteWarsUtility::ImageFormatMap =
{
	{"image/jpeg", EImageFormat::JPEG},
	{"image/png", EImageFormat::PNG},
	{"image/bmp", EImageFormat::BMP}
};

FString AccelByteWarsUtility::GenerateActorEntityId(const AActor* Actor)
{
	if (!Actor) 
	{
		return FString::Printf(TEXT("%s_%d"), *ENTITY_TYPE_UNKNOWN, INDEX_NONE);
	}

	return FString::Printf(TEXT("%s_%d"), *Actor->GetName(), Actor->GetUniqueID());
}

FString AccelByteWarsUtility::FormatEntityDeathSource(const FString& SourceType, const FString& SourceEntityId)
{
	return FString::Printf(TEXT("%s:%s"), *SourceType, *SourceEntityId);
}

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
	FCacheBrush ImageBrush = nullptr;
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
	const FString ProjectVerSectionPath = TEXT("/Script/EngineSettings.GeneralProjectSettings");
	const FString ProjectVerConfig = TEXT("ProjectVersion");

	FString ProjectVersionStr = TEXT("");
	GConfig->GetString(*ProjectVerSectionPath, *ProjectVerConfig, ProjectVersionStr, GGameIni);
	return ProjectVersionStr;
}

void AccelByteWarsUtility::SetGameVersion(const FString& NewGameVersion)
{
	const FString ProjectVerSectionPath = TEXT("/Script/EngineSettings.GeneralProjectSettings");
	const FString ProjectVerConfig = TEXT("ProjectVersion");

	GConfig->SetString(*ProjectVerSectionPath, *ProjectVerConfig, *NewGameVersion, GGameIni);
	GConfig->Flush(false, GGameIni);

	// Reload the config to reflect the changes.
	UGeneralProjectSettings* ProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	if (ProjectSettings)
	{
		ProjectSettings->LoadConfig();
	}
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
	const FString CmdStr = TEXT("-UseVersionChecker=");
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

int32 AccelByteWarsUtility::GetControllerId(const APlayerState* PlayerState)
{
	int32 ControllerId = 0;
	if (const APlayerController* PC = PlayerState->GetPlayerController())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			ControllerId = LP->GetLocalPlayerIndex();
		}
	}
	return ControllerId;
}

int32 AccelByteWarsUtility::GetLocalUserNum(const APlayerController* PC)
{
	int32 LocalUserNum = 0;
	if (PC)
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			LocalUserNum = LP->GetControllerId();
		}
	}
	return LocalUserNum;
}

FUniqueNetIdPtr AccelByteWarsUtility::GetUserId(const APlayerController* PC)
{
	FUniqueNetIdPtr UniqueNetId = nullptr;
	if (PC)
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			UniqueNetId = LP->GetPreferredUniqueNetId().GetUniqueNetId();
		}
	}
	return UniqueNetId;
}

bool AccelByteWarsUtility::GetFlagValueOrDefault(
	const FString& Keyword,
	const FString& ConfigSectionKeyword,
	const bool DefaultValue)
{
	bool bIsActive = DefaultValue;

	// Check launch param
	const FString CmdArgs = FCommandLine::Get();
	const FString CmdKeyword = FString::Printf(TEXT("-%s="), *Keyword);
	if (CmdArgs.Contains(*CmdKeyword, ESearchCase::IgnoreCase))
	{
		FString ValueInString = TEXT("");
		FParse::Value(*CmdArgs, *CmdKeyword, ValueInString);

		if (ValueInString.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase))
		{
			bIsActive = true;
		}
		else if (ValueInString.Equals(TEXT("FALSE"), ESearchCase::IgnoreCase))
		{
			bIsActive = false;
		}
	}
	// Check DefaultEngine.ini
	else
	{
		bIsActive = GConfig->GetBoolOrDefault(*ConfigSectionKeyword, *Keyword, bIsActive, GEngineIni);
	}

	return bIsActive;
}

bool AccelByteWarsUtility::IsValidEmailAddress(const FString& Email)
{
	FRegexPattern EmailPattern(TEXT(R"((^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$))"));
	FRegexMatcher EmailMatcher(EmailPattern, Email);
	return EmailMatcher.FindNext();
}
