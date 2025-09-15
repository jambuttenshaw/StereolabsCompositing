#include "Composure/SlCompElementInput.h"

#include "SlCompEngineSubsystem.h"
#include "Engine/Engine.h"


USlCompInput::USlCompInput()
{
}

FMatrix44f USlCompInput::GetProjectionMatrix() const
{
	if (USlCompEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		return static_cast<FMatrix44f>(Subsystem->GetProjectionMatrix());
	}
	return FMatrix44f::Identity;
}

FMatrix44f USlCompInput::GetInverseProjectionMatrix() const
{
	if (USlCompEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		return static_cast<FMatrix44f>(Subsystem->GetInvProjectionMatrix());
	}
	return FMatrix44f::Identity;
}

float USlCompInput::GetNearClippingPlane() const
{
	if (USlCompEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		return Subsystem->GetNearClippingPlane();
	}
	return 10.0f /* Sensible default*/;
}

UTexture* USlCompInput::GenerateInput_Implementation()
{
	UTexture* Result = nullptr;
	if (USlCompEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
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
