#pragma once

#include "CompositingCaptureBase.h"

#include "SlCompCaptureBase.generated.h"


/**
 *	Base class for CG Compositing elements that will work with Stereolabs Cameras
 */
UCLASS(BlueprintType)
class STEREOLABSCOMPOSITING_API AStereolabsCompositingCaptureBase : public ACompositingCaptureBase
{
	GENERATED_BODY()

public:
	AStereolabsCompositingCaptureBase();

protected:
	// Rendering resources extracted from the scene renderer for use in composition
	// This layer provides a place to keep these resources safe and reference them in later Composure passes,
	// after the scene rendering has been completed

	// For access on render thread only
	TRefCountPtr<IPooledRenderTarget> CachedFroxelGrid;

private:
	TSharedPtr<class FSlCompViewExtension, ESPMode::ThreadSafe> SlCompViewExtension;

public:
	friend class FSlCompViewExtension;
};
