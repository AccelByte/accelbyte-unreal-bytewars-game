// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AccelByteWarsEditorTarget : TargetRules
{
	public AccelByteWarsEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "AccelByteWars", "AccelByteUe4Sdk", "OnlineSubsystemAccelByte", "AccelByteNetworkUtilities" } );
	}
}
