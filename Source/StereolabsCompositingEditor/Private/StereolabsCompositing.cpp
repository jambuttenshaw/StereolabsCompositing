// Copyright Epic Games, Inc. All Rights Reserved.

#include "StereolabsCompositing.h"
#include "StereolabsCompositingStyle.h"
#include "StereolabsCompositingCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"


#define LOCTEXT_NAMESPACE "FStereolabsCompositingModule"

void FStereolabsCompositingModule::StartupModule()
{
	FStereolabsCompositingStyle::Initialize();
	FStereolabsCompositingStyle::ReloadTextures();

	FStereolabsCompositingCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FStereolabsCompositingCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FStereolabsCompositingModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FStereolabsCompositingModule::RegisterMenus));
}

void FStereolabsCompositingModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FStereolabsCompositingStyle::Shutdown();
	FStereolabsCompositingCommands::Unregister();
}

void FStereolabsCompositingModule::PluginButtonClicked()
{
}

void FStereolabsCompositingModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FStereolabsCompositingCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FStereolabsCompositingCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStereolabsCompositingModule, StereolabsCompositing)