// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModules/Module-3/MatchmakingSubsystem_Starter.h"
#include "P2PMatchmakingSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UP2PMatchmakingSubsystem_Starter : public UMatchmakingSubsystem_Starter
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


#pragma region Module.9 Function Declarations

protected:
	// TODO: Add your protected Module.9 function declarations here.

#pragma region Module.9 Function Declarations
};