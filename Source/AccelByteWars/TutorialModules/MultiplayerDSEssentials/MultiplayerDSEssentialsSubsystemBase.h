// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "MultiplayerDSEssentialsSubsystemBase.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API UMultiplayerDSEssentialsSubsystemBase : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool IsAMSServer() const { return false; }
};
