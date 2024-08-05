// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Components/AccelByteWarsButtonBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameUIManagerSubsystem.generated.h"

class ULocalPlayer;
class UGameUIController;
class UCommonLocalPlayer; // TODO @afif change this to UAccelByteWarsLocalPlayer

/**
 * This class managing the UI flow by getting the events from game flow
 * and passing it to the base UI that the game used for managing the hierarchy of the menu system.
 */
UCLASS(Abstract, config = Game)
class ACCELBYTEWARS_API UGameUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGameUIManagerSubsystem() { }
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	const UGameUIController* GetCurrentUIController() const { return CurrentUIController; }
	UGameUIController* GetCurrentUIController() { return CurrentUIController; }

	virtual void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

	void SetSelectedButton(UAccelByteWarsButtonBase* NewSelectedButton);
	void SetHoveredButton(UAccelByteWarsButtonBase* NewHoveredButton);
	UAccelByteWarsButtonBase* GetLastSelectedButton();
	UAccelByteWarsButtonBase* GetLastHoveredButton();

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "GameUIManagerSubsystem")
	void GetLastHoveredButton(UAccelByteWarsButtonBase*& LastHovered);

protected:
	void SwitchToUIController(UGameUIController* InUIController);

private:
	UPROPERTY(Transient)
	UGameUIController* CurrentUIController = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UGameUIController> DefaultUIControllerClass;

	UPROPERTY()
	TObjectPtr<UAccelByteWarsButtonBase> LastSelectedButton = nullptr;

	UPROPERTY()
	TObjectPtr<UAccelByteWarsButtonBase> LastHoveredButton = nullptr;
};
