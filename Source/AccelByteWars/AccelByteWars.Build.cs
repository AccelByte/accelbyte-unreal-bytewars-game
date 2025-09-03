// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Linq;

public class AccelByteWars : ModuleRules
{
	public AccelByteWars(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] 
		{
			"AccelByteWars",
			"AccelByteWars/TutorialModules"
		});

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"ApplicationCore",
			"CommonUI", 
			"CommonInput",
			"GameplayTags",
			"GameplayAbilities",
			"AccelByteUe4Sdk", 
			"AccelByteNetworkUtilities", 
			"OnlineSubsystemAccelByte",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Json",
			"JsonUtilities",
			"HTTP",
			"EngineSettings",
			"AIModule"
		});


		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"InputCore",
			"Slate",
			"SlateCore",
			"RenderCore",
			"DeveloperSettings",
			"EnhancedInput",
			"NetCore",
			"RHI",
			"Projects",
			"Gauntlet",
			"UMG",
			"CommonUI",
			"CommonInput",
			"AudioMixer",
			"NetworkReplayStreaming",
			"AudioModulation",
			"Niagara",
			"ProceduralMeshComponent",
			"MediaAssets",
		});
		
		if (Target.ProjectDefinitions.Contains("PLATFORM_STEAM"))
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"Steamworks",
				"SteamShared"
			});
			PublicDefinitions.Add("PLATFORM_STEAM=1");
		}
		else 
		{
			PublicDefinitions.Add("PLATFORM_STEAM=0");
		}

		// Use Google services for Android
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"OnlineSubsystemGoogle",
				"OnlineSubsystemGooglePlay"
			});
		}
	}
}
