#include "Composure/SlCompElementInput.h"

#include "SlCompEngineSubsystem.h"
#include "Engine/Engine.h"


USlCompInput::USlCompInput()
{
}

bool USlCompInput::GetCameraData(FAuxiliaryCameraData& OutData)
{
	if (USlCompEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		OutData.ViewToNDCMatrix = static_cast<FMatrix44f>(Subsystem->GetProjectionMatrix());
		OutData.NDCToViewMatrix = static_cast<FMatrix44f>(Subsystem->GetInvProjectionMatrix());
		OutData.NearClipPlane = Subsystem->GetNearClippingPlane();
		OutData.HorizontalFOV = Subsystem->GetHorizontalFieldOfView();
		OutData.VerticalFOV = Subsystem->GetVerticalFieldOfView();

		return true;
	}
	return false;
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
