#include "Composure/SlCompElementInput.h"

#include "Engine/Engine.h"
#include "SlCompEngineSubsystem.h"
#include "SlCompHelpers.h"


USlCompInput::USlCompInput()
{
}

bool USlCompInput::GetCameraIntrinsicData(FCompUtilsCameraIntrinsicData& OutData)
{
	return InputType == ESlCompInputType::SlComp_View
		? FSlCompHelpers::GetCameraIntrinsicData(ViewSource, OutData)
		: FSlCompHelpers::GetCameraIntrinsicData(MeasureSource, OutData);
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
