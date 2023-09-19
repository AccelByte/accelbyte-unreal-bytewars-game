// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "TutorialModuleOnlineSession.h"

void UTutorialModuleOnlineSession::RegisterOnlineDelegates()
{
    // Assign associate Tutorial Module based on default object.
    AssociateTutorialModule = GetClass()->GetDefaultObject<UTutorialModuleOnlineSession>()->AssociateTutorialModule;
}

void UTutorialModuleOnlineSession::ClearOnlineDelegates() {}

void UTutorialModuleOnlineSession::ExecuteNextTick(const FSimpleDelegate& Delegate) const
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
}
