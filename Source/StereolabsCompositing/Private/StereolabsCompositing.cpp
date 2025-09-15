// Copyright Epic Games, Inc. All Rights Reserved.

#include "StereolabsCompositing.h"

#include "ISettingsModule.h"
#include "StereolabsCompositingSettings.h"

#define LOCTEXT_NAMESPACE "FNPRToolsModule"

DEFINE_LOG_CATEGORY(LogStereolabsCompositing);


void FStereolabsCompositingModule::StartupModule()
{
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