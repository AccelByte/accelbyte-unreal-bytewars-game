// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

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

public:
	const UAccelByteWarsGlobals* GetCurrentGlobals() const { return CurrentGlobals; }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Globals"))
	UAccelByteWarsGlobals* GetCurrentGlobals() { return CurrentGlobals; }

private:
	UPROPERTY(Transient)
	UAccelByteWarsGlobals* CurrentGlobals = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UAccelByteWarsGlobals> DefaultGlobalsClass;
};
