// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AccelByteWarsTarget : TargetRules
{
	public AccelByteWarsTarget( TargetInfo Target) : base(Target)
	{
		// Setup IWYU and Unity Build
		bEnforceIWYU = false;
		bUseUnityBuild = true;
		bUseAdaptiveUnityBuild = true;
		
		Type = TargetType.Game;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppStandard = CppStandardVersion.Default;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "AccelByteWars" } );
		
		// Enable logging in shipping builds
		if (Configuration == UnrealTargetConfiguration.Shipping)
		{
			bUseLoggingInShipping = true;
		}
	}
}
