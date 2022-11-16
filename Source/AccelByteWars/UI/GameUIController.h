// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameUIController.generated.h"

class ULocalPlayer;
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
	static GameUIControllerClass* GetGameUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIControllerClass>(GetGameUIPolicy(WorldContextObject));
	}

	static UGameUIController* GetGameUIPolicy(const UObject* WorldContextObject);

	virtual UWorld* GetWorld() const override;
	UGameUIManagerSubsystem* GetOwningUIManager() const;
	UAccelByteWarsBaseUI* GetRootLayout(const ULocalPlayer* LocalPlayer) const;

	ELocalMultiplayerInteractionMode GetLocalMultiplayerInteractionMode() const { return LocalMultiplayerInteractionMode; }

	void RequestPrimaryControl(UAccelByteWarsBaseUI* Layout);

protected:
	void AddLayoutToViewport(ULocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);
	void RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);

	virtual void OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);
	virtual void OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);
	virtual void OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout);

	void CreateLayoutWidget(ULocalPlayer* LocalPlayer);
	TSubclassOf<UAccelByteWarsBaseUI> GetLayoutWidgetClass(ULocalPlayer* LocalPlayer);
	
private:

	ELocalMultiplayerInteractionMode LocalMultiplayerInteractionMode = ELocalMultiplayerInteractionMode::PrimaryOnly;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UAccelByteWarsBaseUI> LayoutClass;

	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;
	
	void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	friend class UGameUIManagerSubsystem;
};
