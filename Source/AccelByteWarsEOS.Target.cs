// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System.Collections.Generic;

public class AccelByteWarsEOSTarget : TargetRules
{
    public AccelByteWarsEOSTarget(TargetInfo Target) : base(Target)
    {
        CustomConfig = "EOS";

        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new string[] { "AccelByteWars", "AccelByteUe4Sdk", "OnlineSubsystemAccelByte", "AccelByteNetworkUtilities" });

        // Enable logging in shipping builds
        if (Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = true;
        }
    }
}
