// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

public class BlackBoxSDKPlatform_Linux : BlackBoxSDKPlatform
{
    public BlackBoxSDKPlatform_Linux(ReadOnlyTargetRules inTarget, string inPluginDir, string inEngineDirectory) : base(inTarget, inPluginDir, inEngineDirectory)
    {

    }
    
    private string GetLibsDirPath()
    {
        return Path.Combine(PluginDir, "Libs/x64/Linux");
    }

    private string GetConfigName()
    {
        // "relwithdebinfo" for blackbox release config with debug symbol
        return "relwithdebinfo/";
    }

#if UE_4_22_OR_LATER
    public override List<string> GetPrivateDefinitions()
    {
        return new List<string>
        {
        };
    }
    public override List<string> GetPublicDefinitions()
    {
        return new List<string>
        {
        };
    }
#else
    public override List<string> GetDefinitions()
    {
        return new List<string>
        {
        };
    }
#endif
    public override List<string> GetRuntimeDependencies()
    {
        return new List<string>
        {
            Path.Combine(GetLibsDirPath(), GetConfigName() + "libblackbox-core.so")   
        };
    }

    public override List<string> GetDebugRuntimeDependencies()
    {
        return new List<string>();
    }

    public override List<string> GetSystemRuntimeDependencies()
    {
        return new List<string>();
    }

    public override Dictionary<string, string> GetMustCopyDebugRuntimeDependencies()
    {
        return new Dictionary<string, string>();
    }

    public override Dictionary<string, string> GetMustCopySystemRuntimeDependencies()
    {
        return new Dictionary<string, string>();
    }

    public override List<string> GetPublicAdditionalLibraries()
    {
#if UE_4_22_OR_LATER
        return new List<string>
        {
            Path.Combine(GetLibsDirPath(), GetConfigName() + "libblackbox-core.so")
        };
#else
        return new List<string>
        {
            Path.Combine(GetLibsDirPath(), GetConfigName() + "libblackbox-core.so")
        };
#endif
    }
    public override List<string> GetPublicSystemLibraries()
    {
#if UE_4_22_OR_LATER
        return new List<string>
        {
        };
#else
        return new List<string>
        {
        };
#endif
    }

    public override List<string> GetPublicDelayLoadDLLs()
    {
        return new List<string>
        {
            "libblackbox-core.so"
        };
    }

    public override List<string> GetPlatformSpecificPrivateDependencyModuleNames()
    {
        return new List<string>
        {
        };
    }

    public override List<string> GetPublicSystemIncludeDirs()
    {
        return new List<string>();
    }

    public override List<string> GetPrivateIncludeDirs()
    {
        return new List<string>
        {
        };
    }
    public override List<string> GetEngineThirdPartyPrivateStaticDependencies()
    {
        return new List<string>()
        {
        };
    }
}
