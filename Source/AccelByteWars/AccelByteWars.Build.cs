// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AccelByteWars : ModuleRules
{
	public AccelByteWars(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicIncludePaths.AddRange(new string[] 
		{
			"AccelByteWars"
		});


		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"CommonUI", 
			"CommonInput", 
			"AccelByteUe4Sdk", 
			"AccelByteNetworkUtilities", 
			"OnlineSubsystemAccelByte",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "Json",
            "HTTP"
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
			"ProceduralMeshComponent"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
