#include "Composure/SlCompElementTransforms.h"

#include "RenderGraphBuilder.h"
#include "CompositingElements/ICompositingTextureLookupTable.h"

#include "SlCompEngineSubsystem.h"
#include "Composure/SlCompCaptureBase.h"
#include "Pipelines/StereolabsCompositingPipelines.h"


///////////////////////////////////////////////
// UCompositingStereolabsDepthProcessingPass //
///////////////////////////////////////////////

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

			auto Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>();
			Params.InvProjectionMatrix = FMatrix44f(Subsystem->GetInvProjectionMatrix());

			Params.bEnableJacobiSteps = bEnableJacobi;
			Params.NumJacobiSteps = NumJacobiSteps;

			Params.bEnableFarClipping = bEnableFarClipping;
			Params.FarClipDistance = FarClipDistance;

			Params.bEnableClippingPlane = bEnableFloorClipping;
			// Construct user clipping plane
			// THESE DIRECTIONS / POSITIONS USE Y AXIS AS UP/DOWN AND Z AXIS AS FRONT/BACK
			FPlane ClippingPlane{ FVector{ 0.0f, -FloorClipDistance, 0.0f }, FVector{ 0.0f, 1.0f, 0.0f } };
			Params.UserClippingPlane = FVector4f{
				static_cast<float>(ClippingPlane.X),
				static_cast<float>(ClippingPlane.Y),
				static_cast<float>(ClippingPlane.Z),
				static_cast<float>(ClippingPlane.W)
			};

			ENQUEUE_RENDER_COMMAND(ApplyNPRTransform)(
				[this, Parameters = MoveTemp(Params), InputResource = Input->GetResource(), OutputResource = RenderTarget->GetResource()]
				(FRHICommandListImmediate& RHICmdList)
				{
					FRDGBuilder GraphBuilder(RHICmdList);

					TRefCountPtr<IPooledRenderTarget> InputRT = CreateRenderTarget(InputResource->GetTextureRHI(), TEXT("StereolabsDepthProcessingPass.Input"));
					TRefCountPtr<IPooledRenderTarget> OutputRT = CreateRenderTarget(OutputResource->GetTextureRHI(), TEXT("StereolabsDepthProcessingPass.Output"));

					// Set up RDG resources
					FRDGTextureRef InColorTexture = GraphBuilder.RegisterExternalTexture(InputRT);
					FRDGTextureRef OutColorTexture = GraphBuilder.RegisterExternalTexture(OutputRT);

					// Execute pipeline
					StereolabsCompositing::ExecuteDepthProcessingPipeline(
						GraphBuilder,
						Parameters,
						InColorTexture,
						OutColorTexture
					);

					GraphBuilder.Execute();
				});

			Result = RenderTarget;
		}
	}

	return Result;
}



///////////////////////////////////////////
// UCompositingStereolabsVolumetricsPass //
///////////////////////////////////////////


UTexture* UCompositingStereolabsVolumetricsPass::ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera)
{
	UTexture* Result = Input;

	if (!Input || !StereolabsCGLayer.IsValid())
		return Result;

	// Validate input colour texture
	check(Input->GetResource());

	FIntPoint Dims;
	Dims.X = Input->GetResource()->GetSizeX();
	Dims.Y = Input->GetResource()->GetSizeY();

	// Get a render target to output to
	UTextureRenderTarget2D* RenderTarget = RequestRenderTarget(Dims, PF_FloatRGBA);
	if (!(RenderTarget && RenderTarget->GetResource()))
		return Result;

	FVolumetricsCompositionParametersProxy Params;
	Params.VolumetricFogData = static_cast<const AStereolabsCompositingCaptureBase*>(StereolabsCGLayer.Get())->GetVolumetricFogData();
	if (!Params.VolumetricFogData || !Params.VolumetricFogData->IntegratedLightScatteringTexture)
		return Result;

	// Get the output of the depth pass
	bool bSuccess = PrePassLookupTable->FindNamedPassResult(DepthPassName, Params.CameraDepthTexture);
	if (!bSuccess || !Params.CameraDepthTexture)
		return Result;

	ENQUEUE_RENDER_COMMAND(ApplyNPRTransform)(
		[this, Parameters = MoveTemp(Params), InputResource = Input->GetResource(), OutputResource = RenderTarget->GetResource()]
		(FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			TRefCountPtr<IPooledRenderTarget> InputRT = CreateRenderTarget(InputResource->GetTextureRHI(), TEXT("StereolabsVolumetricsPass.Input"));
			TRefCountPtr<IPooledRenderTarget> OutputRT = CreateRenderTarget(OutputResource->GetTextureRHI(), TEXT("StereolabsVolumetricsPass.Output"));

			// Set up RDG resources
			FRDGTextureRef InColorTexture = GraphBuilder.RegisterExternalTexture(InputRT);
			FRDGTextureRef OutColorTexture = GraphBuilder.RegisterExternalTexture(OutputRT);

			// Execute pipeline
			StereolabsCompositing::ExecuteVolumetricsCompositionPipeline(
				GraphBuilder,
				Parameters,
				InColorTexture,
				OutColorTexture
			);

			GraphBuilder.Execute();
		});
	Result = RenderTarget;

	return Result;
}
