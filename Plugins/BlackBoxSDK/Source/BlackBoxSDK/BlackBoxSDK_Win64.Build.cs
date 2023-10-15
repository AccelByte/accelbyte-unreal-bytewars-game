// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

public class BlackBoxSDKPlatform_Win64 : BlackBoxSDKPlatform
{
    public BlackBoxSDKPlatform_Win64(ReadOnlyTargetRules inTarget, string inPluginDir, string inEngineDirectory) : base(inTarget, inPluginDir, inEngineDirectory)
    {

    }

    private string GetHelperDirPath()
    {
        return Path.Combine(PluginDir, "Helper/x64");
    }

    private string GetIssueReporterDirPath()
    {
        return Path.Combine(PluginDir, "Issue_Reporter/x64");
    }

    private string GetDLLsDirPath()
    {
        return Path.Combine(PluginDir, "DLLs/x64/Win");
    }

    private string GetLibsDirPath()
    {
        return Path.Combine(PluginDir, "Libs/x64/Win");
    }

    private string GetConfigName()
    {
        // "relwithdebinfo" for blackbox release config with debug symbol
        return "relwithdebinfo/";
    }

    private bool IsReleaseCandidate()
    {
        string GameConfigPath = Path.Combine(PluginDir, "../../Config");
        if (File.Exists(GameConfigPath + "/BlackBox.ini"))
        {
            string EnableFieldWithSpaces = "enable = ";
            string EnableFieldWithoutSpaces = "enable=";

            foreach (string line in File.ReadLines(GameConfigPath + "/BlackBox.ini"))
            {
                var LowerCaseLine = line.ToLower().Trim();

                var FoundWithSpaces = LowerCaseLine.StartsWith(EnableFieldWithSpaces);
                var FoundWithoutSpaces = LowerCaseLine.StartsWith(EnableFieldWithoutSpaces);

                if (FoundWithSpaces || FoundWithoutSpaces)
                {
                    string EnableFieldString = FoundWithSpaces ? EnableFieldWithSpaces : EnableFieldWithoutSpaces;
                    var IsEnabled = LowerCaseLine.Substring(EnableFieldString.Length);

                    if (IsEnabled == "true")
                    {
                        // SDK enabled means it's NOT a release candidate
                        return false;
                    }
                    else
                    {
                        // SDK disabled means it's a release candidate
                        return true;
                    }
                }
            }
        }
        return false;
    }

#if UE_4_22_OR_LATER
    public override List<string> GetPrivateDefinitions()
    {
        return new List<string>
        {
            "BLACKBOX_USE_SHARED_LIBRARY"
        };
    }
    public override List<string> GetPublicDefinitions()
    {
        return new List<string>
        {
            "_CRT_SECURE_NO_WARNINGS"
        };
    }
#else
    public override List<string> GetDefinitions()
    {
        return new List<string>
        {
            "_CRT_SECURE_NO_WARNINGS",
            "BLACKBOX_USE_SHARED_LIBRARY"
        };
    }
#endif
    public override List<string> GetRuntimeDependencies()
    {
        List<string> Deps = new List<string> { };
        if (!IsReleaseCandidate())
        {
            Deps.AddRange(new List<string> {
                Path.Combine(GetHelperDirPath(), "blackbox_helper.exe"),
                Path.Combine(GetHelperDirPath(), "blackbox_helper.ini"),
                Path.Combine(GetIssueReporterDirPath(), "blackbox_issue_reporter.exe")
            });
        }
        Deps.Add(Path.Combine(GetDLLsDirPath(), GetConfigName() + "blackbox-core.dll"));
        return Deps;
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
            Path.Combine(GetLibsDirPath(), GetConfigName() + "blackbox-core.lib")
        };
#else
        return new List<string>
        {
            Path.Combine(GetLibsDirPath(), GetConfigName() + "blackbox-core.lib"),
            "Shlwapi.lib"
        };
#endif
    }
    public override List<string> GetPublicSystemLibraries()
    {
#if UE_4_22_OR_LATER
        return new List<string>
        {
            "Shlwapi.lib"
        };
#else
        return new List<string>();
#endif
    }

    public override List<string> GetPublicDelayLoadDLLs()
    {
        return new List<string>
        {
            "blackbox-core.dll"
        };
    }

    public override List<string> GetPlatformSpecificPrivateDependencyModuleNames()
    {
        return new List<string>
        {
#if UE_5_0_OR_LATER
            "RHICore",
#endif
            "D3D11RHI",
            "D3D12RHI"
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
            Path.Combine(EngineDirectoryDir,"Source/Runtime/D3D12RHI/Private"),
            Path.Combine(EngineDirectoryDir,"Source/Runtime/D3D12RHI/Private/Windows"),
        };
    }
    public override List<string> GetEngineThirdPartyPrivateStaticDependencies()
    {
        List<string> BlackBoxPrivateDependencies = new List<string>()
        {
#if UE_5_0_OR_LATER
            "RHICore",
#endif
            "DX12",
            "NVAPI",
            "NVAftermath"
        };
#if UE_4_26_OR_LATER
        if (Target.bBuildEditor)
        {
            string[] UTraceDependencies = {
                "TraceLog",
                "TraceAnalysis"
            };
            BlackBoxPrivateDependencies.AddRange(UTraceDependencies);
        }
#endif
        return BlackBoxPrivateDependencies;
    }
}