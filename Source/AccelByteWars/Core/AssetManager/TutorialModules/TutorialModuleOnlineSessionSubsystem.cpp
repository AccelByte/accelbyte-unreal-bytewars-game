// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "TutorialModuleOnlineSessionSubsystem.h"

#include "TutorialModuleOnlineSession.h"

bool UTutorialModuleOnlineSessionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const TSubclassOf<UTutorialModuleOnlineSession> ModuleOnlineSession = GetOnlineSessionClass();
	const TSubclassOf<UOnlineSession> CurrentOnlineSession =
		GEngine->GetWorldFromContextObject(Outer, EGetWorldErrorMode::Assert)->GetGameInstance()->GetOnlineSession()->GetClass();

	// safety. turn off subsystem if one of the online session is nullptr
	if (ModuleOnlineSession == nullptr || CurrentOnlineSession == nullptr)
	{
		return false;
	}

	// if current online session class is not the exact same as module's online session class, disable subsystem
	const bool bIsTheSameClass = CurrentOnlineSession == ModuleOnlineSession;
	return bIsTheSameClass ? Super::ShouldCreateSubsystem(Outer) : false;
}
