// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System.Collections.Generic;

public class AccelByteWarsSteamTarget : AccelByteWarsTarget
{
	public AccelByteWarsSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
		ProjectDefinitions.Add("PLATFORM_STEAM");
	}
}
