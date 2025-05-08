#pragma once

#include "CoreMinimal.h"

#include "CompositingElements/CompositingElementPasses.h"
#include "Pipelines/StereolabsCompositingPipelines.h"

#include "StereolabsCompositingElementTransforms.generated.h"


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API UCompositingStereolabsDepthRelaxationPass : public UCompositingElementTransform
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter="PassName", EditCondition="bEnabled"))
	bool bEnableJacobi = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition="bEnabled"))
	int32 NumJacobiPasses = 16;

public:
	virtual UTexture* ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera) override;

private:

	void ApplyTransform_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureResource* InputResource, FTextureResource* RenderTargetResource) const;

private:
	// Only access on render thread!
	DepthRelaxationParameters Parameters_RenderThread;
};
