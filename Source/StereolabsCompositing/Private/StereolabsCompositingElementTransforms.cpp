#include "StereolabsCompositingElementTransforms.h"


#include "RenderGraphBuilder.h"


UTexture* UCompositingStereolabsDepthProcessingPass::ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera)
{
	UTexture* Result = Input;

	if (Input)
	{
		// Validate input colour texture
		check(Input->GetResource());

		FIntPoint Dims;
		Dims.X = Input->GetResource()->GetSizeX();
		Dims.Y = Input->GetResource()->GetSizeY();

		// Get a render target to output to
		UTextureRenderTarget2D* RenderTarget = RequestRenderTarget(Dims, PF_FloatRGBA);
		if (RenderTarget && RenderTarget->GetResource())
		{
			FDepthProcessingParametersProxy Params;
			Params.bEnableJacobiSteps = false;
			Params.NumJacobiSteps = 0;
			Params.FarClipDistance = FarClipDistance;

			ENQUEUE_RENDER_COMMAND(ApplyNPRTransform)(
				[this, TempParams = MoveTemp(Params), InputResource = Input->GetResource(), OutputResource = RenderTarget->GetResource()]
				(FRHICommandListImmediate& RHICmdList)
				{
					this->Parameters_RenderThread = TempParams;
					this->ApplyTransform_RenderThread(RHICmdList, InputResource, OutputResource);
				});

			Result = RenderTarget;
		}
	}

	return Result;
}


void UCompositingStereolabsDepthProcessingPass::ApplyTransform_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureResource* InputResource, FTextureResource* RenderTargetResource) const
{
	check(IsInRenderingThread());

	FRDGBuilder GraphBuilder(RHICmdList);

	TRefCountPtr<IPooledRenderTarget> InputRT = CreateRenderTarget(InputResource->GetTextureRHI(), TEXT("StereolabsDepthProcessingPass.Input"));
	TRefCountPtr<IPooledRenderTarget> OutputRT = CreateRenderTarget(RenderTargetResource->GetTextureRHI(), TEXT("StereolabsDepthProcessingPass.Output"));

	// Set up RDG resources
	FRDGTextureRef InColorTexture = GraphBuilder.RegisterExternalTexture(InputRT);
	FRDGTextureRef OutColorTexture = GraphBuilder.RegisterExternalTexture(OutputRT);

	// Execute pipeline
	StereolabsCompositing::ExecuteDepthProcessingPipeline(
		GraphBuilder,
		Parameters_RenderThread,
		InColorTexture,
		OutColorTexture
	);

	GraphBuilder.Execute();
}
