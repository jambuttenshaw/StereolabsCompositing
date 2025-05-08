// Copyright Epic Games, Inc. All Rights Reserved.

#include "StereolabsCompositing.h"

#define LOCTEXT_NAMESPACE "FNPRToolsModule"

DEFINE_LOG_CATEGORY(LogStereolabsCompositing);


void FStereolabsCompositingModule::StartupModule()
{
	// Stereolabs module is not set to load on PostConfigInit, so shader source mapping is not set at correct time.
	// Instead of modifying that module I will just map shader source here
	//FString PluginDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("Stereolabs"));
	//FString ShaderDirectory = FPaths::Combine(PluginDir, TEXT("Shaders"));
	//AddShaderSourceDirectoryMapping("/Plugin/Stereolabs", ShaderDirectory);
}

void FStereolabsCompositingModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStereolabsCompositingModule, StereolabsCompositing)