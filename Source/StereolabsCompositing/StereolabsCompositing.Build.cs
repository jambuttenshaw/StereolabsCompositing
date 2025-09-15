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
                "Composure",
                "CompositionUtils"
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

        // Require private access to renderer for using volumetric fog
        string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);

        PrivateIncludePaths.AddRange(
	        new string[]
	        {
		        EnginePath + "Source/Runtime/Renderer/Private/",
		        EnginePath + "Source/Runtime/Renderer/Internal/",
	        }
        );
    }
}
