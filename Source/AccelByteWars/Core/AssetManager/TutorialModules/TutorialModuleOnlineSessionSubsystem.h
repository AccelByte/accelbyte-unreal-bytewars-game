// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModuleSubsystem.h"
#include "TutorialModuleOnlineSessionSubsystem.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API UTutorialModuleOnlineSessionSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	/**
	 * Use this instead of getting the class from AssociateTutorialModule since AssociateTutorialModule set on
	 * Initialize and this function will be called before that. 
	 */
	virtual TSubclassOf<UTutorialModuleOnlineSession> GetOnlineSessionClass() const { return nullptr; }
};
