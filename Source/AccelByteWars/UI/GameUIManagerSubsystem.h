// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameUIManagerSubsystem.generated.h"

class ULocalPlayer;
class UGameUIController;
class ULocalPlayer; // TODO @afif change this to UAccelByteWarsLocalPlayer

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

	virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	virtual void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

protected:
	void SwitchToUIController(UGameUIController* InUIController);

private:
	UPROPERTY(Transient)
	UGameUIController* CurrentUIController = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UGameUIController> DefaultUIControllerClass;
};
