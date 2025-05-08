//======= Copyright (c) Stereolabs Corporation, All rights reserved. ===============

using UnrealBuildTool;
using System.IO;
using System;

public class StereolabsCompositing : ModuleRules
{
    public StereolabsCompositing(ReadOnlyTargetRules Target) : base(Target)
    {
	    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Stereolabs",
                "Composure"
			}
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "RenderCore",
				"Renderer",
                "RHI",
                "RHICore"
            }
        );
    }
}
