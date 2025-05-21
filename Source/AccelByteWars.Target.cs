// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AccelByteWarsTarget : TargetRules
{
	public AccelByteWarsTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppStandard = CppStandardVersion.Default;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "AccelByteWars", "AccelByteUe4Sdk", "OnlineSubsystemAccelByte", "AccelByteNetworkUtilities" } );
		
		// Enable logging in shipping builds
		if (Configuration == UnrealTargetConfiguration.Shipping)
		{
			bUseLoggingInShipping = true;
		}
	}
}
