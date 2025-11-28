#include "ReprojectionCalibrationStereolabsTarget.h"

#include "SlCompEngineSubsystem.h"


#if WITH_EDITOR

void UReprojectionCalibrationStereolabsTarget::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if we need to request a new source from the engine subsystem
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	bool TargetChanged = PropertyName == GET_MEMBER_NAME_CHECKED(UReprojectionCalibrationStereolabsTarget, InputType);
	TargetChanged	  |= PropertyName == GET_MEMBER_NAME_CHECKED(UReprojectionCalibrationStereolabsTarget, ViewSource);
	TargetChanged	  |= PropertyName == GET_MEMBER_NAME_CHECKED(UReprojectionCalibrationStereolabsTarget, MeasureSource);
	TargetChanged	  |= PropertyName == GET_MEMBER_NAME_CHECKED(UReprojectionCalibrationStereolabsTarget, bInverseTonemapping);
	if (TargetChanged)
	{
		FetchNewWrapper();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

TObjectPtr<UTexture> UReprojectionCalibrationStereolabsTarget::GetTexture()
{
	if (!ImageWrapper.IsValid())
	{
		FetchNewWrapper();
	}

	if (!ImageWrapper.IsValid())
	{
		return nullptr;
	}

	return ImageWrapper->GetTexture();
}

void UReprojectionCalibrationStereolabsTarget::FetchNewWrapper()
{
	if (auto Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		switch (InputType)
		{
			case ESlCompInputType::SlComp_View:		ImageWrapper = Subsystem->GetOrCreateImageWrapper(ViewSource, bInverseTonemapping);		break;
			case ESlCompInputType::SlComp_Measure:	ImageWrapper = Subsystem->GetOrCreateImageWrapper(MeasureSource);						break;
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
