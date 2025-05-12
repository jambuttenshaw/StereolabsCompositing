#include "Composure/SlCompElementInput.h"

#include "SlCompEngineSubsystem.h"
#include "Engine/Engine.h"


USlCompInput::USlCompInput()
{
}


UTexture* USlCompInput::GenerateInput_Implementation()
{
	USlCompEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>();
	UTexture* Result = nullptr;

	if (Subsystem)
	{
		switch (InputSource)
		{
		case ESlCompInputChannel::Color: Result = Subsystem->GetColorTexture(); break;
		case ESlCompInputChannel::Depth: Result = Subsystem->GetDepthTexture(); break;
		case ESlCompInputChannel::Normal: Result = Subsystem->GetNormalTexture(); break;
		}
	}

	return Result;
}
