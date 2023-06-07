// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModules/Module-3/MatchmakingEssentialsSubsystem.h"
#include "P2PMatchmakingSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UP2PMatchmakingSubsystem : public UMatchmakingEssentialsSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool TravelClient(FName SessionName, APlayerController* PC) override;
};