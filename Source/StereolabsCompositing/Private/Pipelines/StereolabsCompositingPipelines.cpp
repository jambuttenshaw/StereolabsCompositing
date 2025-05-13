#include "StereolabsCompositingPipelines.h"

#include "ScreenPass.h"
#include "StereolabsCompositingShaders.h"

DECLARE_GPU_STAT_NAMED(SlCompDepthProcessingStat, TEXT("SlCompDepthProcessing"));
DECLARE_GPU_STAT_NAMED(SlCompVolumetricCompositionStat, TEXT("SlCompVolumetricComposition"));


namespace StereolabsCompositing{
template <typename Shader, typename SamplerState = TStaticSamplerState<SF_Bilinear>>
void AddPass(
	FRDGBuilder& GraphBuilder,
	FRDGEventName&& PassName,
	FRDGTextureRef RenderTarget,
	std::function<void(typename Shader::FParameters*)>&& SetPassParametersLambda,
	typename Shader::FPermutationDomain Permutation = TShaderPermutationDomain(),
	FIntRect OutRect = FIntRect(),
	FIntRect InRect = FIntRect()
)
{
	FScreenPassTextureViewport OutViewPort = OutRect.IsEmpty() ?
		FScreenPassTextureViewport{ RenderTarget->Desc.Extent } :
		FScreenPassTextureViewport{ RenderTarget->Desc.Extent, OutRect };
	FScreenPassTextureViewport InViewPort = InRect.IsEmpty() ?
		FScreenPassTextureViewport{ RenderTarget->Desc.Extent } :
		FScreenPassTextureViewport{ RenderTarget->Desc.Extent, InRect };

	typename Shader::FParameters* PassParameters = GraphBuilder.AllocParameters<typename Shader::FParameters>();
	PassParameters->OutViewPort = GetScreenPassTextureViewportParameters(OutViewPort);
	PassParameters->InViewPort = GetScreenPassTextureViewportParameters(InViewPort);
	PassParameters->sampler0 = SamplerState::GetRHI(); // bilinear clamped sampler

	SetPassParametersLambda(PassParameters);
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTarget, ERenderTargetLoadAction::ENoAction);

	const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<Shader> PixelShader(ShaderMap, Permutation);

	AddDrawScreenPass(
		GraphBuilder,
		std::move(PassName),
		FScreenPassViewInfo{ GMaxRHIFeatureLevel },
		OutViewPort,
		InViewPort,
		PixelShader,
		PassParameters
	);
}
}


FRDGTextureRef CreateTextureFrom(FRDGBuilder& GraphBuilder, FRDGTextureRef InTex, const TCHAR* Name, float ScaleFactor = 1.0f)
{
	FRDGTextureDesc Desc = InTex->Desc;
	Desc.ClearValue = FClearValueBinding(FLinearColor(0.0f, 0.0f, 0.0f));
	Desc.Format = PF_FloatRGBA;
	Desc.Extent.X = static_cast<int>(static_cast<float>(Desc.Extent.X) * ScaleFactor);
	Desc.Extent.Y = static_cast<int>(static_cast<float>(Desc.Extent.Y) * ScaleFactor);
	return GraphBuilder.CreateTexture(Desc, Name);
}


void StereolabsCompositing::ExecuteDepthProcessingPipeline(
	FRDGBuilder& GraphBuilder, 
	const FDepthProcessingParametersProxy& Parameters,
	FRDGTextureRef InTexture,
	FRDGTextureRef OutTexture
)
{
	check(IsInRenderingThread());

	RDG_EVENT_SCOPE_STAT(GraphBuilder, SlCompDepthProcessingStat, "SlCompDepthProcessing");
	RDG_GPU_STAT_SCOPE(GraphBuilder, SlCompDepthProcessingStat);
	SCOPED_NAMED_EVENT(SlCompDepthProcessing, FColor::Purple);

	FRDGTextureRef TempTexture1 = CreateTextureFrom(GraphBuilder, OutTexture, TEXT("StereolabsCompositingDepthProcessing.Temp1"));
	FRDGTextureRef TempTexture2 = CreateTextureFrom(GraphBuilder, OutTexture, TEXT("StereolabsCompositingDepthProcessing.Temp2"));

	StereolabsCompositing::AddPass<FPreProcessDepthPS>(
		GraphBuilder,
		RDG_EVENT_NAME("PreProcessDepth"),
		TempTexture1,
		[&](auto PassParameters)
		{
			PassParameters->InTex = GraphBuilder.CreateSRV(InTexture);
		}
	);

	if (Parameters.bEnableJacobiSteps)
	{
		for (uint32 i = 0; i < Parameters.NumJacobiSteps; i++)
		{
			StereolabsCompositing::AddPass<FJacobiStepPS, TStaticSamplerState<>>(
				GraphBuilder,
				RDG_EVENT_NAME("JacobiStep(i=%d)", 2*i),
				TempTexture2,
				[&](auto PassParameters)
				{
					PassParameters->InTex = GraphBuilder.CreateSRV(TempTexture1);
				}
			);

			StereolabsCompositing::AddPass<FJacobiStepPS, TStaticSamplerState<>>(
				GraphBuilder,
				RDG_EVENT_NAME("JacobiStep(i=%d)", 2*i+1),
				TempTexture1,
				[&](auto PassParameters)
				{
					PassParameters->InTex = GraphBuilder.CreateSRV(TempTexture2);
				}
			);
		}
	}

	// Post Processing
	StereolabsCompositing::AddPass<FDepthClippingPS>(
		GraphBuilder,
		RDG_EVENT_NAME("DepthClipping"),
		OutTexture,
		[&](auto PassParameters)
		{
			PassParameters->InvCameraProjectionMatrix = Parameters.InvProjectionMatrix;
			PassParameters->UserClippingPlane = Parameters.UserClippingPlane;
			PassParameters->FarClipDistance = Parameters.FarClipDistance;

			PassParameters->InTex = GraphBuilder.CreateSRV(TempTexture1);
		}
	);
}


void StereolabsCompositing::ExecuteVolumetricsCompositionPipeline(
	FRDGBuilder& GraphBuilder, 
	const FVolumetricsCompositionParametersProxy& Parameters, 
	FRDGTextureRef InTexture, 
	FRDGTextureRef OutTexture
)
{
	check(IsInRenderingThread());
	check(Parameters.IsValid());

	RDG_EVENT_SCOPE_STAT(GraphBuilder, SlCompVolumetricCompositionStat, "SlCompVolumetricComposition");
	RDG_GPU_STAT_SCOPE(GraphBuilder, SlCompVolumetricCompositionStat);
	SCOPED_NAMED_EVENT(SlCompVolumetricCompositionStat, FColor::Purple);

	FRDGTextureRef IntegratedLightScatteringTexture = GraphBuilder.RegisterExternalTexture(Parameters.VolumetricFogData->IntegratedLightScatteringTexture);

	StereolabsCompositing::AddPass<FVolumetricCompositionPS, TStaticSamplerState<SF_Bilinear>>(
		GraphBuilder,
		RDG_EVENT_NAME("SlCompVolumetricComposition"),
		OutTexture,
		[&](auto PassParameters)
		{
			PassParameters->CameraColorTexture = GraphBuilder.CreateSRV(InTexture);
			PassParameters->CameraDepthTexture = Parameters.CameraDepthTexture->GetResource()->TextureRHI;

			PassParameters->IntegratedLightScattering = GraphBuilder.CreateSRV(IntegratedLightScatteringTexture);
			PassParameters->IntegratedLightScatteringSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

			PassParameters->VolumetricFogStartDistance = Parameters.VolumetricFogData->VolumetricFogStartDistance;
			PassParameters->VolumetricFogInvGridSize = Parameters.VolumetricFogData->VolumetricFogInvGridSize;
			PassParameters->VolumetricFogGridZParams = Parameters.VolumetricFogData->VolumetricFogGridZParams;
			PassParameters->VolumetricFogSVPosToVolumeUV = Parameters.VolumetricFogData->VolumetricFogSVPosToVolumeUV;
			PassParameters->VolumetricFogUVMax = Parameters.VolumetricFogData->VolumetricFogUVMax;
			PassParameters->OneOverPreExposure = Parameters.VolumetricFogData->OneOverPreExposure;
		}
	);
}
