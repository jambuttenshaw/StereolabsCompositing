// Copyright Epic Games, Inc. All Rights Reserved.

#include "StereolabsCompositingCommands.h"

#define LOCTEXT_NAMESPACE "FStereolabsCompositingModule"

void FStereolabsCompositingCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "StereolabsCompositing", "Execute StereolabsCompositing action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
