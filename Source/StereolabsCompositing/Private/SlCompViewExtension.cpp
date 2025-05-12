#include "SlCompViewExtension.h"

#include "RenderGraphBuilder.h"
#include "SceneRendering.h"
#include "StereolabsCompositing.h"

#include "Composure/SlCompCaptureBase.h"


BEGIN_SHADER_PARAMETER_STRUCT(FCopyFroxelParameters, )
	RDG_TEXTURE_ACCESS(Input, ERHIAccess::CopySrc)
	RDG_TEXTURE_ACCESS(Output, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()


FSlCompViewExtension::FSlCompViewExtension(const FAutoRegister& AutoRegister, AStereolabsCompositingCaptureBase* Owner)
	: FSceneViewExtensionBase(AutoRegister)
	, CaptureActor(Owner)
{
}

void FSlCompViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
}

void FSlCompViewExtension::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& View)
{
	if (!View.bIsSceneCapture)
	{
		return;
	}

	// Make sure this is only active when rendering to a SlCompCaptureBase
	if (!CaptureActor.IsValid())
	{
		return;
	}
	const AStereolabsCompositingCaptureBase* SceneCapture = CaptureActor.Get();
	TRefCountPtr<IPooledRenderTarget>& FroxelBufferResource = const_cast<TRefCountPtr<IPooledRenderTarget>&>(SceneCapture->CachedFroxelGrid);

	// Safely cast
	if (!View.bIsViewInfo)
	{
		return;
	}
	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);

	// Get volumetric fog froxel buffer
	FRDGTextureRef InputTexture = ViewInfo.VolumetricFogResources.IntegratedLightScatteringTexture;
	if (!InputTexture)
	{
		// No texture to extract
		return;
	}

	FRDGTextureRef OutputTexture = FroxelBufferResource.IsValid()
								? GraphBuilder.RegisterExternalTexture(FroxelBufferResource)
								: GraphBuilder.CreateTexture(InputTexture->Desc, TEXT("FroxelBufferCopy"));

	{
		FCopyFroxelParameters* Parameters = GraphBuilder.AllocParameters<FCopyFroxelParameters>();
		Parameters->Input = InputTexture;
		Parameters->Output = OutputTexture;

		// Copy resource
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("CopyFroxelBuffer"),
			Parameters,
			ERDGPassFlags::Copy | ERDGPassFlags::NeverCull,
			[InputTexture, OutputTexture](FRHICommandList& RHICommandList)
		{
			RHICommandList.CopyTexture(
				InputTexture->GetRHI(),
				OutputTexture->GetRHI(),
				FRHICopyTextureInfo()
			);
		});
	}

	GraphBuilder.QueueTextureExtraction(OutputTexture, &FroxelBufferResource);
}
