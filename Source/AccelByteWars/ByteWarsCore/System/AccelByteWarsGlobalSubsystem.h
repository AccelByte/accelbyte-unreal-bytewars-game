// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccelByteWarsGlobalSubsystem.generated.h"

class UAccelByteWarsGlobals;

/**
 * 
 */
UCLASS(config = Game)
class ACCELBYTEWARS_API UAccelByteWarsGlobalSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UAccelByteWarsGlobalSubsystem() { }
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	const UAccelByteWarsGlobals* GetCurrentGlobals() const { return CurrentGlobals; }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Globals"))
	UAccelByteWarsGlobals* GetCurrentGlobals() { return CurrentGlobals; }

private:
	UPROPERTY(Transient)
	UAccelByteWarsGlobals* CurrentGlobals = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UAccelByteWarsGlobals> DefaultGlobalsClass;
};
