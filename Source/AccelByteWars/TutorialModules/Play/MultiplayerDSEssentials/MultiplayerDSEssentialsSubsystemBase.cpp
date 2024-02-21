// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MultiplayerDSEssentialsSubsystemBase.h"

bool UMultiplayerDSEssentialsSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
{
#pragma region "Check if using AMS or not"
	bool bUseAMS = true; // default is true

	// Check launch param. Prioritize launch param.
	FString UseAMSString;
	if (FParse::Value(FCommandLine::Get(), TEXT("-bServerUseAMS="), UseAMSString))
	{
		bUseAMS = !UseAMSString.Equals("false", ESearchCase::Type::IgnoreCase);
	}
	// check DefaultEngine.ini next
	else
	{
		GConfig->GetBool(TEXT("/ByteWars/TutorialModule.DSEssentials"), TEXT("bServerUseAMS"), bUseAMS, GEngineIni);
	}
#pragma endregion

	return Super::ShouldCreateSubsystem(Outer) ? IsAMSServer() == bUseAMS : false;
}
