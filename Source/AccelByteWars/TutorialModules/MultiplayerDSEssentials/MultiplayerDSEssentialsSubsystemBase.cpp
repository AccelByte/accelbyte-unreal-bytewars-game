// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MultiplayerDSEssentialsSubsystemBase.h"

bool UMultiplayerDSEssentialsSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
{
	// check launch param
	bool bUseAMS = FParse::Param(FCommandLine::Get(), TEXT("-ServerUseAMS"));

	// check DefaultEngine.ini next
	if (!bUseAMS)
	{
		FString Config;
		GConfig->GetBool(TEXT("/ByteWars/TutorialModule.DSEssentials"), TEXT("bServerUseAMS"), bUseAMS, GEngineIni);
	}

	return Super::ShouldCreateSubsystem(Outer) ? IsAMSServer() == bUseAMS : false;
}
