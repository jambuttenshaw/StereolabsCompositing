#pragma once

#include "CompositingCaptureBase.h"

#include "SlCompCaptureBase.generated.h"


struct FVolumetricFogRequiredData;

/**
 *	Base class for CG Compositing elements that will work with Stereolabs Cameras
 */
UCLASS(BlueprintType)
class STEREOLABSCOMPOSITING_API AStereolabsCompositingCaptureBase : public ACompositingCaptureBase
{
	GENERATED_BODY()

public:
	AStereolabsCompositingCaptureBase();

	const FVolumetricFogRequiredData* GetVolumetricFogData() const;
protected:
	FVolumetricFogRequiredData* GetVolumetricFogData();

	// Rendering resources extracted from the scene renderer for use in composition
	// This layer provides a place to keep these resources safe and reference them in later Composure passes,
	// after the scene rendering has been completed
	TSharedPtr<struct FVolumetricFogRequiredData> VolumetricFogData_RenderThread;

private:
	TSharedPtr<class FSlCompViewExtension, ESPMode::ThreadSafe> SlCompViewExtension;

	friend class FSlCompViewExtension;
};
