#pragma once

#include "CoreMinimal.h"
#include "ReprojectionCalibration.h"

#include "Core/StereolabsBaseTypes.h"
#include "SlCompTypes.h"

#include "SlCompEngineSubsystem.h"

#include "ReprojectionCalibrationStereolabsTarget.generated.h"


/**
 * Stereolabs Compositing: Represents an image from Stereolabs ZED camera as a target for reprojection
 */
UCLASS()
class STEREOLABSCOMPOSITING_API UReprojectionCalibrationStereolabsTarget : public UReprojectionCalibrationTargetBase
{
	GENERATED_BODY()

private:
	// Private as modifying these members will change which ImageWrapper this input element should point to
	// TODO: Add UFUNCTION's to allow blueprint to interface

	UPROPERTY(EditAnywhere)
	ESlCompInputType InputType = ESlCompInputType::SlComp_View;

	UPROPERTY(EditAnywhere)
	ESlView ViewSource = ESlView::V_Left;

	UPROPERTY(EditAnywhere)
	ESlMeasure MeasureSource = ESlMeasure::M_Depth;

protected:
	//~ Begin UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject interface

public:
	//~ Begin UReprojectionCalibrationTargetBase interface
	virtual TObjectPtr<UTexture> GetTexture() override;
	//~ End UReprojectionCalibrationTargetBase interface

private:
	// Wrapper is left as nullptr if failed
	void FetchNewWrapper();

private:
	TSharedPtr<FSlCompImageWrapper> ImageWrapper;

};
