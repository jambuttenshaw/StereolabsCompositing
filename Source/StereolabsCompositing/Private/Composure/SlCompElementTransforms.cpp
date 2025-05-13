#include "Composure/SlCompElementTransforms.h"

#include "RenderGraphBuilder.h"
#include "CompositingElements/ICompositingTextureLookupTable.h"

#include "SlCompEngineSubsystem.h"
#include "Composure/SlCompCaptureBase.h"
#include "Pipelines/StereolabsCompositingPipelines.h"

#include "Components/DirectionalLightComponent.h"


///////////////////////////////////////////////
// UCompositingStereolabsDepthProcessingPass //
///////////////////////////////////////////////

UTexture* UCompositingStereolabsDepthProcessingPass::ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera)
{
	if (!Input)
		return Input;
	check(Input->GetResource());

	FIntPoint Dims;
	Dims.X = Input->GetResource()->GetSizeX();
	Dims.Y = Input->GetResource()->GetSizeY();

	UTextureRenderTarget2D* RenderTarget = RequestRenderTarget(Dims, PF_FloatRGBA);
	if (!(RenderTarget && RenderTarget->GetResource()))
		return Input;

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

	ENQUEUE_RENDER_COMMAND(ApplyDepthProcessingPass)(
		[Parameters = MoveTemp(Params), InputResource = Input->GetResource(), OutputResource = RenderTarget->GetResource()]
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

	return RenderTarget;
}



///////////////////////////////////////////
// UCompositingStereolabsVolumetricsPass //
///////////////////////////////////////////


UTexture* UCompositingStereolabsVolumetricsPass::ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera)
{
	if (!Input || !StereolabsCGLayer.IsValid())
		return Input;
	check(Input->GetResource());

	FIntPoint Dims;
	Dims.X = Input->GetResource()->GetSizeX();
	Dims.Y = Input->GetResource()->GetSizeY();

	UTextureRenderTarget2D* RenderTarget = RequestRenderTarget(Dims, PF_FloatRGBA);
	if (!(RenderTarget && RenderTarget->GetResource()))
		return Input;

	FVolumetricsCompositionParametersProxy Params;
	Params.VolumetricFogData = static_cast<const AStereolabsCompositingCaptureBase*>(StereolabsCGLayer.Get())->GetVolumetricFogData();
	if (!Params.VolumetricFogData || !Params.VolumetricFogData->IntegratedLightScatteringTexture)
		return Input;

	// Get the output of the depth pass
	bool bSuccess = PrePassLookupTable->FindNamedPassResult(CameraDepthPassName, Params.CameraDepthTexture);
	if (!bSuccess || !Params.CameraDepthTexture)
		return Input;

	ENQUEUE_RENDER_COMMAND(ApplyVolumetricCompositionPass)(
		[Parameters = MoveTemp(Params), InputResource = Input->GetResource(), OutputResource = RenderTarget->GetResource()]
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

	return RenderTarget;
}


///////////////////////////////////////////
// UCompositingStereolabsVolumetricsPass //
///////////////////////////////////////////


UTexture* UCompositingStereolabsRelightingPass::ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera)
{
	if (!Input)
		return Input;
	check(Input->GetResource());

	if (!TargetCamera || !TargetCamera->GetCameraComponent())
		return Input;

	FIntPoint Dims;
	Dims.X = Input->GetResource()->GetSizeX();
	Dims.Y = Input->GetResource()->GetSizeY();

	UTextureRenderTarget2D* RenderTarget = RequestRenderTarget(Dims, PF_FloatRGBA);
	if (!(RenderTarget && RenderTarget->GetResource()))
		return Input;

	FRelightingParametersProxy Params;
	bool bSuccess = PrePassLookupTable->FindNamedPassResult(CameraDepthPassName, Params.CameraDepthTexture);
	bSuccess &= PrePassLookupTable->FindNamedPassResult(CameraNormalPassName, Params.CameraNormalTexture);

	Params.LightProxy = (LightSource.IsValid() && LightSource->GetComponent()) ? LightSource->GetComponent()->SceneProxy : nullptr;

	// Get the camera view matrix
	FMinimalViewInfo CameraView;
	TargetCamera->GetCameraComponent()->GetCameraView(0.0f, CameraView);
	Params.CameraTransform.SetRotation(CameraView.Rotation.Quaternion());
	Params.CameraTransform.SetTranslation(CameraView.Location);

	Params.VirtualLightWeight = VirtualLightWeight;
	Params.RealLightWeight = RealLightWeight;

	if (!bSuccess || !Params.IsValid())
		return Input;

	ENQUEUE_RENDER_COMMAND(ApplyRelightingPass)(
		[Parameters = MoveTemp(Params), InputResource = Input->GetResource(), OutputResource = RenderTarget->GetResource()]
		(FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			TRefCountPtr<IPooledRenderTarget> InputRT = CreateRenderTarget(InputResource->GetTextureRHI(), TEXT("StereolabsRelightingPass.Input"));
			TRefCountPtr<IPooledRenderTarget> OutputRT = CreateRenderTarget(OutputResource->GetTextureRHI(), TEXT("StereolabsRelightingPass.Output"));

			// Set up RDG resources
			FRDGTextureRef InColorTexture = GraphBuilder.RegisterExternalTexture(InputRT);
			FRDGTextureRef OutColorTexture = GraphBuilder.RegisterExternalTexture(OutputRT);

			// Execute pipeline
			StereolabsCompositing::ExecuteRelightingPipeline(
				GraphBuilder,
				Parameters,
				InColorTexture,
				OutColorTexture
			);

			GraphBuilder.Execute();
		});

	return RenderTarget;
}
