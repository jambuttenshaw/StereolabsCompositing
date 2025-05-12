#include "StereolabsCompositingElementInput.h"

#include "StereolabsCompositingEngineSubsystem.h"
#include "Engine/Engine.h"


UStereolabsCompositingInput::UStereolabsCompositingInput()
{
}


UTexture* UStereolabsCompositingInput::GenerateInput_Implementation()
{
	UStereolabsCompositingEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<UStereolabsCompositingEngineSubsystem>();
	UTexture* Result = nullptr;

	if (Subsystem)
	{
		switch (InputSource)
		{
		case EStereolabsCompositingInputChannel::Color: Result = Subsystem->GetColorTexture(); break;
		case EStereolabsCompositingInputChannel::Depth: Result = Subsystem->GetDepthTexture(); break;
		case EStereolabsCompositingInputChannel::Normal: Result = Subsystem->GetNormalTexture(); break;
		}
	}

	return Result;
}
