// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TutorialModuleSubsystem.generated.h"

class UTutorialModuleDataAsset;

UCLASS(Abstract)
class ACCELBYTEWARS_API UTutorialModuleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	bool ShouldCreateSubsystem(UObject* Outer) const override;
	UTutorialModuleDataAsset* AssociateTutorialModule;
};