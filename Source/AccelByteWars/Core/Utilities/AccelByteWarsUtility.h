// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"

#define UNREAL_ENGINE_VERSION_OLDER_THAN_5_2 (ENGINE_MAJOR_VERSION <= 4 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 1))

typedef TSharedPtr<const FSlateBrush> FCacheBrush;
DECLARE_DELEGATE_OneParam(FOnImageReceived, FCacheBrush);

// All possible Reason to be used for the FOnMatchEnded delegate.
#define GAME_END_REASON_TERMINATED FString(TEXT("Terminated"))
#define GAME_END_REASON_TIMES_UP FString(TEXT("Time is over"))
#define GAME_END_REASON_LAST_TEAM FString(TEXT("One team remains"))

// All possible DestroyedType to be used for the FOnEntityDestroyed delegate.
#define ENTITY_DESTROYED_TYPE_HIT_SHIP FString(TEXT("ShipHit"))
#define ENTITY_DESTROYED_TYPE_HIT_PLANET FString(TEXT("PlanetHit"))
#define ENTITY_DESTROYED_TYPE_HIT_LIFETIME FString(TEXT("Lifetime"))
#define ENTITY_DESTROYED_TYPE_HIT_POWERUP FString(TEXT("Powerup"))

// All possible EntityType to be used for the FOnEntityDestroyed delegate.
#define ENTITY_TYPE_MISSILE FString(TEXT("Missile"))
#define ENTITY_TYPE_PLANET FString(TEXT("Planet"))
#define ENTITY_TYPE_POWERUP FString(TEXT("Powerup"))
#define ENTITY_TYPE_PLAYER FString(TEXT("Player"))
#define ENTITY_TYPE_UNKNOWN FString(TEXT("Unknown"))

/**
 * Delegate for tracking player entering match and current player when a match (the actual game play, after the initial 5 seconds delay) started.
 * @param PlayerNetId Unique net ID of the player
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredMatch, const FUniqueNetIdPtr /*PlayerNetId*/)

/**
 * Delegate for tracking when a match ended (just as the game over screen about to popped up).
 * @param Reason The reason why the match ends.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, const FString& /*Reason*/)

/**
 * Delegate for tracking entity destruction. Entity can be player and missile.
 * @param EntityType Entity type that is currently being destroyed. Player of Missile.
 * @param DestroyedPlayerId Unique Net ID of the owning player if the destroyed entity is a player possessed ship. Empty otherwhise.
 * @param DestroyedEntityId In-game ID of the entity destroyed.
 * @param DestroyedType What causes the destruction.
 * @param SourceEntityId In-game ID of the entity that causing the destruction.
 */
DECLARE_MULTICAST_DELEGATE_SixParams(
	FOnEntityDestroyed,
	const FString& /*EntityType*/,
	const FUniqueNetIdPtr /*DestroyedPlayerId*/,
	const FString& /*DestroyedEntityId*/,
	const FVector& /*DestroyedLocation*/,
	const FString& /*DestroyedType*/,
	const FString& /*SourceEntityId*/)

/**
 * Delegate for tracking when a player used a power up.
 * @param MissileOwnerNetId Unique Net ID of the player that used the power up.
 * @param ItemName In-game item ID of the power up used.
 * @param UsedAmount How many of the power up item does the player used in this one activation.
 * @param AmountLeft How many of the power up item does the player still has in its inventory.
 */
DECLARE_MULTICAST_DELEGATE_FourParams(
	FOnPowerUpUsed,
	const FUniqueNetIdPtr /*MissileOwnerNetId*/,
	const FString& /*ItemName*/,
	const int /*UsedAmount*/,
	const int /*AmountLeft*/)

class UCommonUserWidget;
class APlayerState;

#define FLAG_GUI_CHEAT TEXT("GUICheatOverride")
#define FLAG_GUI_CHEAT_SECTION TEXT("AccelByteTutorialModules")

class ACCELBYTEWARS_API AccelByteWarsUtility
{
public:
	static FString GenerateActorEntityId(const AActor* Actor);
	static FString FormatEntityDeathSource(const FString& SourceType, const FString& SourceEntityId);

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

	/**
	 * @brief Get launch parameter value in float on launch param (first prio) or DefaultEngine.ini (second prio). Return DefaultValue if not set or set to non number.
	 * @param Keyword Launch parameter keyword. Setting this as "param" will make the function look for "-param=" in the launch param
	 * @param ConfigSectionKeyword DefaultEngine.ini [section] where the flag located
	 * @param DefaultValue Default Value if flag not found anywhere
	 * @return Launch parameter value
	 */
	static float GetLaunchParamFloatValueOrDefault(const FString& Keyword, const FString& ConfigSectionKeyword, const float DefaultValue);

	static bool IsValidEmailAddress(const FString& Email);

private:
	static const TMap<FString, EImageFormat> ImageFormatMap;
};