// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "StereolabsCompositingStyle.h"

class FStereolabsCompositingCommands : public TCommands<FStereolabsCompositingCommands>
{
public:

	FStereolabsCompositingCommands()
		: TCommands<FStereolabsCompositingCommands>(TEXT("StereolabsCompositing"), NSLOCTEXT("Contexts", "StereolabsCompositing", "StereolabsCompositing Plugin"), NAME_None, FStereolabsCompositingStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
