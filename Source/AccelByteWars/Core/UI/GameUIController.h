// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameUIController.generated.h"

class ULocalPlayer;
class UCommonLocalPlayer;
class UAccelByteWarsBaseUI;
class UGameUIManagerSubsystem;

UENUM()
enum class ELocalMultiplayerInteractionMode : uint8
{
	// Fullscreen viewport for the primary player only, regardless of the other player's existence
	PrimaryOnly,

	// Fullscreen viewport for one player, but players can swap control over who's is displayed and who's is dormant
	SingleToggle,

	// Viewports displayed simultaneously for both players
	Simultaneous
};

USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(Transient)
	ULocalPlayer* LocalPlayer = nullptr;

	UPROPERTY(Transient)
	UAccelByteWarsBaseUI* RootLayout = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FRootViewportLayoutInfo() {}
	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UAccelByteWarsBaseUI* InRootLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		, RootLayout(InRootLayout)
		, bAddedToViewport(bIsInViewport)
	{}

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};
/**
 * 
 */
UCLASS(Abstract, Blueprintable, Within = GameUIManagerSubsystem)
class ACCELBYTEWARS_API UGameUIController : public UObject
{
	GENERATED_BODY()

public:
	template <typename GameUIControllerClass = UGameUIController>
	static GameUIControllerClass* GetGameUIControllerAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIControllerClass>(GetGameUIController(WorldContextObject));
	}

	static UGameUIController* GetGameUIController(const UObject* WorldContextObject);

	virtual UWorld* GetWorld() const override;
	UGameUIManagerSubsystem* GetOwningUIManager() const;
	UAccelByteWarsBaseUI* GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const;

	ELocalMultiplayerInteractionMode GetLocalMultiplayerInteractionMode() const { return LocalMultiplayerInteractionMode; }

	void RequestPrimaryControl(UAccelByteWarsBaseUI* Layout);

protected:
	void AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);
	void RemoveLayoutFromViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);

	virtual void OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);
	virtual void OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);
	virtual void OnRootLayoutReleased(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);

	void CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer);
	TSubclassOf<UAccelByteWarsBaseUI> GetLayoutWidgetClass(UCommonLocalPlayer* LocalPlayer);
	
private:

	ELocalMultiplayerInteractionMode LocalMultiplayerInteractionMode = ELocalMultiplayerInteractionMode::PrimaryOnly;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UAccelByteWarsBaseUI> LayoutClass;

	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;
	
	void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

	friend class UGameUIManagerSubsystem;
};
