#pragma once

#include "CoreMinimal.h"

#include "CompositingElements/CompositingElementPasses.h"
#include "Pipelines/StereolabsCompositingPipelines.h"

#include "SlCompElementTransforms.generated.h"


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API UCompositingStereolabsDepthProcessingPass : public UCompositingElementTransform
{
	GENERATED_BODY()

public:

	// Reconstruction Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	bool bEnableJacobi = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	int32 NumJacobiSteps = 10.0f;

	// Clipping Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	float FarClipDistance = 200.0f; // 200cm

	// Height of camera above floor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	float FloorClipDistance = 100.0f; // 100cm

public:
	virtual UTexture* ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera) override;

private:

	void ApplyTransform_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureResource* InputResource, FTextureResource* RenderTargetResource) const;

private:
	// Only access on render thread!
	FDepthProcessingParametersProxy Parameters_RenderThread;
};


/**
 * Composites volumetric fog from the scene onto the camera image, using the real-world depth
 */
UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API UCompositingStereolabsVolumetricsPass : public UCompositingElementTransform
{		
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	FName DepthPassName;

	/** Used to get resources from the scene renderer that are required for composing volumetric effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	TWeakObjectPtr<class AStereolabsCompositingCaptureBase> StereolabsCGLayer;

public:
	virtual UTexture* ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera) override;

};
