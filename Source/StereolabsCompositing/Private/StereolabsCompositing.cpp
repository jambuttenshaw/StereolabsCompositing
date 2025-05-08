// Copyright Epic Games, Inc. All Rights Reserved.

#include "StereolabsCompositing.h"
#include "StereolabsCompositingStyle.h"
#include "StereolabsCompositingCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName StereolabsCompositingTabName("StereolabsCompositing");

#define LOCTEXT_NAMESPACE "FStereolabsCompositingModule"

void FStereolabsCompositingModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
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
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FStereolabsCompositingStyle::Shutdown();

	FStereolabsCompositingCommands::Unregister();
}

void FStereolabsCompositingModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FStereolabsCompositingModule::PluginButtonClicked()")),
							FText::FromString(TEXT("StereolabsCompositing.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
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