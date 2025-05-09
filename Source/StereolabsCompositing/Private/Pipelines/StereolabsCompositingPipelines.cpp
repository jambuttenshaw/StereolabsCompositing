#include "StereolabsCompositingPipelines.h"

#include "ScreenPass.h"
#include "StereolabsCompositingShaders.h"


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

	FRDGTextureRef TempTexture = CreateTextureFrom(GraphBuilder, OutTexture, TEXT("StereolabsCompositingDepthProcessing.Temp"));

	StereolabsCompositing::AddPass<FPreProcessDepthPS>(
		GraphBuilder,
		RDG_EVENT_NAME("PreProcessDepth"),
		TempTexture,
		[&](auto PassParameters)
		{
			PassParameters->InTex = GraphBuilder.CreateSRV(InTexture);
		}
	);

	/*
	if (Parameters.bEnableJacobiSteps)
	{

		for (uint32 i = 0; i < Parameters.NumJacobiSteps; i++)
		{
			StereolabsCompositing::AddPass<FJacobiStepPS>(
				GraphBuilder,
				RDG_EVENT_NAME("JacobiStep(i=%d)", 2*i),
				PingTexture,
				[&](auto PassParameters)
				{
					PassParameters->InTex = GraphBuilder.CreateSRV(OutTexture);
				}
			);

			StereolabsCompositing::AddPass<FJacobiStepPS>(
				GraphBuilder,
				RDG_EVENT_NAME("JacobiStep(i=%d)", 2*i+1),
				OutTexture,
				[&](auto PassParameters)
				{
					PassParameters->InTex = GraphBuilder.CreateSRV(PingTexture);
				}
			);
		}
	}
	*/

	// Post Processing
	StereolabsCompositing::AddPass<FDepthClippingPS>(
		GraphBuilder,
		RDG_EVENT_NAME("DepthClipping"),
		OutTexture,
		[&](auto PassParameters)
		{
			PassParameters->FarClipDistance = Parameters.FarClipDistance;
			PassParameters->InTex = GraphBuilder.CreateSRV(TempTexture);
		}
	);
}
