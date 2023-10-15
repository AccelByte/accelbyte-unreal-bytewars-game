// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

public abstract class BlackBoxSDKPlatform
{
    protected ReadOnlyTargetRules Target;
    protected string PluginDir;
    protected string EngineDirectoryDir;

    public BlackBoxSDKPlatform(ReadOnlyTargetRules inTarget, string inPluginDir, string inEngineDirectory)
    {
        Target = inTarget;
        PluginDir = inPluginDir;
        EngineDirectoryDir = inEngineDirectory;
    }

    public static BlackBoxSDKPlatform GetPlatformInstance(ReadOnlyTargetRules Target, string PluginDir, string inEngineDirectory)
    {
        var platformType = System.Type.GetType("BlackBoxSDKPlatform_" + Target.Platform.ToString());
        if (platformType == null)
        {
            throw new BuildException("BlackBoxSDK does not support platform " + Target.Platform.ToString());
        }
        var platformInstance = Activator.CreateInstance(platformType, Target, PluginDir, inEngineDirectory) as BlackBoxSDKPlatform;
        if (platformInstance == null)
        {
            throw new BuildException("Cannot instantiate BlackBoxSDK for platform " + Target.Platform.ToString());
        }
        return platformInstance;
    }

#if UE_4_22_OR_LATER
    public abstract List<string> GetPrivateDefinitions();
    public abstract List<string> GetPublicDefinitions();
#else
    public abstract List<string> GetDefinitions();
#endif
    public abstract List<string> GetRuntimeDependencies();
    public abstract List<string> GetDebugRuntimeDependencies();
    public abstract List<string> GetSystemRuntimeDependencies();
    public abstract Dictionary<string, string> GetMustCopyDebugRuntimeDependencies();
    public abstract Dictionary<string, string> GetMustCopySystemRuntimeDependencies();
    public abstract List<string> GetPublicAdditionalLibraries();
    public abstract List<string> GetPublicDelayLoadDLLs();
    public abstract List<string> GetPlatformSpecificPrivateDependencyModuleNames();
    public abstract List<string> GetPublicSystemIncludeDirs();
    public abstract List<string> GetPublicSystemLibraries();
    public abstract List<string> GetPrivateIncludeDirs();
    public abstract List<string> GetEngineThirdPartyPrivateStaticDependencies();
}

public class BlackBoxSDK : ModuleRules
{
    public BlackBoxSDK(ReadOnlyTargetRules Target) : base(Target)
    {
        string BlackBoxPluginDir = Path.Combine(ModuleDirectory, "../../");
        BlackBoxSDKPlatform platformInstance = BlackBoxSDKPlatform.GetPlatformInstance(Target, BlackBoxPluginDir, EngineDirectory);

        PrivatePCHHeaderFile = "Public/BlackBoxPch.h";
        bEnableExceptions = true;
        bAllowConfidentialPlatformDefines = true;



#if UE_4_22_OR_LATER
        foreach(var def in platformInstance.GetPublicDefinitions())
        {
            PublicDefinitions.Add(def);
        }
        foreach(var def in platformInstance.GetPrivateDefinitions())
        {
            PrivateDefinitions.Add(def);
        }
        CppStandard = CppStandardVersion.Cpp17;
        PCHUsage = ModuleRules.PCHUsageMode.NoSharedPCHs;
        PrivateDependencyModuleNames.AddRange(
            new string[] {
				"Core",
                "CoreUObject",
                "Engine",
                "ImageWrapper",
                "RenderCore",
                "RHI",
                "Slate",
                "SlateCore",
                "InputCore",
                "HTTP"
            }
        );
#else
        foreach (var def in platformInstance.GetDefinitions())
        {
            Definitions.Add(def);
        }
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "ImageWrapper",
                "RenderCore",
                "ShaderCore",
                "RHI",
                "Slate",
                "SlateCore",
                "InputCore",
                "HTTP"
            }
        );
#endif
        // Uncomment the following to enable crash report only and without other functions
        // Refer to Source/BlackBoxSDK/Private/BlackBoxFeatureCompileSwitches.h 
        // for the possible list of definitions and their effects
        //PrivateDefinitions.Add("BLACKBOX_SDK_MINIMAL_CRASH_REPORT_ONLY=1");

        PrivateDependencyModuleNames.AddRange(
            platformInstance.GetPlatformSpecificPrivateDependencyModuleNames().ToArray()
        );

        foreach (var path in platformInstance.GetRuntimeDependencies())
        {
            RuntimeDependencies.Add(path, StagedFileType.NonUFS);
        }
        foreach (var path in platformInstance.GetDebugRuntimeDependencies())
        {
            RuntimeDependencies.Add(path, StagedFileType.DebugNonUFS);
        }
        foreach (var path in platformInstance.GetSystemRuntimeDependencies())
        {
            RuntimeDependencies.Add(path, StagedFileType.SystemNonUFS);
        }
        foreach (KeyValuePair<string, string> paths in platformInstance.GetMustCopySystemRuntimeDependencies())
        {
            RuntimeDependencies.Add(paths.Key, paths.Value, StagedFileType.SystemNonUFS);
        }
        foreach (KeyValuePair<string, string> paths in platformInstance.GetMustCopyDebugRuntimeDependencies())
        {
            RuntimeDependencies.Add(paths.Key, paths.Value, StagedFileType.DebugNonUFS);
        }
        foreach (var path in platformInstance.GetPublicAdditionalLibraries())
        {
            PublicAdditionalLibraries.Add(path);
        }
        foreach (var dllName in platformInstance.GetPublicDelayLoadDLLs())
        {
            PublicDelayLoadDLLs.Add(dllName);
        }
        foreach (var sysLibName in platformInstance.GetPublicSystemLibraries())
        {
            PublicSystemLibraries.Add(sysLibName);
        }
        foreach (var path in platformInstance.GetPublicSystemIncludeDirs())
        {
            PublicSystemIncludePaths.Add(path);
        }
        foreach (var path in platformInstance.GetPrivateIncludeDirs())
        {
            PrivateIncludePaths.Add(path);
        }
        foreach (var libname in platformInstance.GetEngineThirdPartyPrivateStaticDependencies())
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, libname);
        }
    }
}
