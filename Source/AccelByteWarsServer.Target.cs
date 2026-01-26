// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AccelByteWarsServerTarget : TargetRules
{
	public AccelByteWarsServerTarget( TargetInfo Target) : base(Target)
	{
		// Setup IWYU and Unity Build
		bEnforceIWYU = false;
		bUseUnityBuild = true;
		bUseAdaptiveUnityBuild = true;
		
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "AccelByteWars" } );
	}
}
