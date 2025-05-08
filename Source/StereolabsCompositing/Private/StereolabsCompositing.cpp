// Copyright Epic Games, Inc. All Rights Reserved.

#include "StereolabsCompositing.h"

#include "ISettingsModule.h"
#include "StereolabsCompositingSettings.h"

#define LOCTEXT_NAMESPACE "FNPRToolsModule"

DEFINE_LOG_CATEGORY(LogStereolabsCompositing);


void FStereolabsCompositingModule::StartupModule()
{
	FString PluginDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("StereolabsCompositing"));
	FString ShaderDirectory = FPaths::Combine(PluginDir, TEXT("Shaders"));
	AddShaderSourceDirectoryMapping("/Plugin/StereolabsCompositing", ShaderDirectory);

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Stereolabs Settings",
			LOCTEXT("RuntimeSettingsName", "Stereolabs"), LOCTEXT("RuntimeSettingsDescription", "Configure ZED Camera Settings"),
			GetMutableDefault<UStereolabsCompositingSettings>());

	}
}

void FStereolabsCompositingModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStereolabsCompositingModule, StereolabsCompositing)