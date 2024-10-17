// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"

typedef TSharedPtr<const FSlateBrush> FCacheBrush;
DECLARE_DELEGATE_OneParam(FOnImageReceived, FCacheBrush);

class UCommonUserWidget;
class APlayerState;

#define FLAG_GUI_CHEAT TEXT("GUICheatOverride")
#define FLAG_GUI_CHEAT_SECTION TEXT("AccelByteTutorialModules")

class ACCELBYTEWARS_API AccelByteWarsUtility
{
public:
	static void GetImageFromURL(const FString& Url, const FString& ImageId, const FOnImageReceived& OnReceived);
	static FCacheBrush GetImageFromCache(const FString& ImageId);

	/** @brief Always return positive value for Dividend % Modulus. If Modulus is zero, returns -1 as to prevent divide by zero exception. */
	static int32 PositiveModulo(const int32 Dividend, const int32 Modulus);

	static FString GetGameVersion();
	static void SetGameVersion(const FString& NewGameVersion);

	static TArray<UUserWidget*> FindWidgetsOnTheScreen(
		const FString& WidgetName, 
		const TSubclassOf<UUserWidget> WidgetClass, 
		const bool bTopLevelOnly, 
		UObject* Context);
	
	static bool IsUseVersionChecker();
	static int32 GetControllerId(const APlayerState* PlayerState);
	static int32 GetLocalUserNum(const APlayerController* PC);
	static FUniqueNetIdPtr GetUserId(const APlayerController* PC);

	/**
	 * @brief Get flag's value is set to TRUE / FALSE on launch param (first prio) or DefaultEngine.ini (second prio). Return DefaultValue if not set
	 * @param Keyword Flag keyword. Setting this as "flag" will make the function look for "-flag" in the launch param
	 * @param ConfigSectionKeyword DefaultEngine.ini [section] where the flag located
	 * @param DefaultValue Default Value if flag not found anywhere
	 * @return Whether the flag is set as true or false
	 */
	static bool GetFlagValueOrDefault(const FString& Keyword, const FString& ConfigSectionKeyword, const bool DefaultValue);

	static bool IsValidEmailAddress(const FString& Email);

private:
	static const TMap<FString, EImageFormat> ImageFormatMap;
};