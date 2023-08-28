// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionClient.h"
#include "TutorialModuleOnlineSession.generated.h"

class UTutorialModuleDataAsset;

UCLASS(Abstract)
class ACCELBYTEWARS_API UTutorialModuleOnlineSession : public UOnlineSessionClient
{
	GENERATED_BODY()

public:
	// The Tutorial Module Data Asset associated with this subsystem.
	UTutorialModuleDataAsset* AssociateTutorialModule;

	virtual void RegisterOnlineDelegates() override;

protected:
	void ExecuteNextTick(const FSimpleDelegate& Delegate) const;
	virtual void JoinSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult) override {}
};
