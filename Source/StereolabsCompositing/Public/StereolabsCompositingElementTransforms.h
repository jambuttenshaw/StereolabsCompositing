#pragma once

#include "CoreMinimal.h"

#include "CompositingElements/CompositingElementPasses.h"
#include "Pipelines/StereolabsCompositingPipelines.h"

#include "StereolabsCompositingElementTransforms.generated.h"


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API UCompositingStereolabsDepthProcessingPass : public UCompositingElementTransform
{
	GENERATED_BODY()

public:

	// Reconstruction Parameters

	// Clipping Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	float FarClipDistance = 200.0f; // 200cm

public:
	virtual UTexture* ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera) override;

private:

	void ApplyTransform_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureResource* InputResource, FTextureResource* RenderTargetResource) const;

private:
	// Only access on render thread!
	FDepthProcessingParametersProxy Parameters_RenderThread;
};
