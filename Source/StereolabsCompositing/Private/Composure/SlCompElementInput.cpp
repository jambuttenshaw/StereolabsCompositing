#include "Composure/SlCompElementInput.h"

#include "SlCompEngineSubsystem.h"
#include "Engine/Engine.h"


USlCompInput::USlCompInput()
{
}

bool USlCompInput::GetCameraIntrinsicData(FCompUtilsCameraIntrinsicData& OutData)
{
	if (USlCompEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		OutData.ViewToNDC = static_cast<FMatrix44f>(Subsystem->GetProjectionMatrix());
		OutData.NDCToView = static_cast<FMatrix44f>(Subsystem->GetInvProjectionMatrix());
		OutData.HorizontalFOV = Subsystem->GetHorizontalFieldOfView();
		OutData.VerticalFOV = Subsystem->GetVerticalFieldOfView();

		return true;
	}
	return false;
}

UTexture* USlCompInput::GenerateInput_Implementation()
{
	if (!ImageWrapper)
	{
		FetchNewWrapper();
	}

	return ImageWrapper ? ImageWrapper->GetTexture() : nullptr;
}

#if WITH_EDITOR

void USlCompInput::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if we need to request a new source from the engine subsystem

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	bool TargetChanged = PropertyName == GET_MEMBER_NAME_CHECKED(USlCompInput, InputType);
	TargetChanged	  |= PropertyName == GET_MEMBER_NAME_CHECKED(USlCompInput, ViewSource);
	TargetChanged	  |= PropertyName == GET_MEMBER_NAME_CHECKED(USlCompInput, MeasureSource);
	if (TargetChanged)
	{
		FetchNewWrapper();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void USlCompInput::FetchNewWrapper()
{
	if (auto Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		switch (InputType)
		{
			case ESlCompInputType::SlComp_View:		ImageWrapper = Subsystem->GetOrCreateImageWrapper(ViewSource);		break;
			case ESlCompInputType::SlComp_Measure:	ImageWrapper = Subsystem->GetOrCreateImageWrapper(MeasureSource);	break;
			default:
			{
				checkNoEntry();
			}
		}
	}
	else
	{
		ImageWrapper = nullptr;
	}
}
