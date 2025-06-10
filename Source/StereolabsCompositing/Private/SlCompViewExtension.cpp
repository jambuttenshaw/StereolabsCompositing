#include "SlCompViewExtension.h"

#include "Composure/SlCompCaptureBase.h"
#include "ScreenPass.h"

#include "RenderGraphBuilder.h"
#include "SceneRendering.h"

class FCameraFeedInjectionPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCameraFeedInjectionPS);
	SHADER_USE_PARAMETER_STRUCT(FCameraFeedInjectionPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)

		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InputViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutputViewPort)

		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, CameraColorTexture) // Not RDG resources
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, CameraDepthTexture) 
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, CameraNormalsTexture)

		SHADER_PARAMETER(FMatrix44f, CameraLocalToWorld)

		SHADER_PARAMETER(float, AlbedoMultiplier)
		SHADER_PARAMETER(float, AmbientMultiplier)

		SHADER_PARAMETER(float, RoughnessOverride)
		SHADER_PARAMETER(float, SpecularOverride)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment
	)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		// We need to set these so that the shader generation utils will set up the GBuffer writing defines
		// TODO: This will probably cause all sorts of problems if also using substrate
		// TODO: because the render target layout is different
		OutEnvironment.SetDefine(TEXT("IS_BASE_PASS"), true);
		OutEnvironment.SetDefine(TEXT("MATERIALBLENDING_SOLID"), true);
		OutEnvironment.SetDefine(TEXT("MATERIAL_SHADINGMODEL_DEFAULT_LIT"), true);
	}
};

IMPLEMENT_GLOBAL_SHADER(FCameraFeedInjectionPS, "/Plugin/StereolabsCompositing/CameraFeedInjection.usf", "InjectCameraFeedPS", SF_Pixel)


FSlCompViewExtension::FSlCompViewExtension(AStereolabsCompositingCaptureBase* Owner)
	: CaptureActor(Owner)
{
	check(Owner);
}


void FSlCompViewExtension::PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView, const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures)
{
	check(InView.bIsSceneCapture && InView.bIsViewInfo && CaptureActor.IsValid());

	if (CaptureActor->bInjectionMode)
	{
		InjectCameraFeed(GraphBuilder, InView);
	}
}

void FSlCompViewExtension::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& View)
{
	check(View.bIsSceneCapture && View.bIsViewInfo && CaptureActor.IsValid());

	if (!CaptureActor->bInjectionMode || CaptureActor->bExtractVolumetricFogInInjectionMode)
	{
		ExtractVolumetricFog(GraphBuilder, View);
	}
}


void FSlCompViewExtension::InjectCameraFeed(FRDGBuilder& GraphBuilder, FSceneView& InView) const
{
	check(InView.bIsSceneCapture && InView.bIsViewInfo && CaptureActor.IsValid());
	FViewInfo& ViewInfo = static_cast<FViewInfo&>(InView);

	// Get camera images and insert them into GBuffer
	// Some of these textures may be nullptr, must check for that later
	const FStereolabsCameraTexturesProxy& CameraTextures = CaptureActor->GetCameraTextures_RenderThread();

	bool bAllValid = CameraTextures.ColorTexture != nullptr
		&& CameraTextures.DepthTexture != nullptr
		&& CameraTextures.NormalsTexture != nullptr;

	if (!bAllValid)
	{
		return;
	}

	{
		FCameraFeedInjectionPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FCameraFeedInjectionPS::FParameters>();

		PassParameters->View = InView.ViewUniformBuffer;

		PassParameters->CameraColorTexture = CameraTextures.ColorTexture->GetResource()->TextureRHI;
		PassParameters->CameraDepthTexture = CameraTextures.DepthTexture->GetResource()->TextureRHI;
		PassParameters->CameraNormalsTexture = CameraTextures.NormalsTexture->GetResource()->TextureRHI;

		FTransform CameraTransform = CaptureActor->GetCameraTransform();
		PassParameters->CameraLocalToWorld = static_cast<FMatrix44f>(CameraTransform.ToMatrixNoScale());

		PassParameters->AmbientMultiplier = CaptureActor->AmbientMultiplier;
		PassParameters->AlbedoMultiplier = CaptureActor->AlbedoMultiplier;

		PassParameters->RoughnessOverride = CaptureActor->RoughnessOverride;
		PassParameters->SpecularOverride = CaptureActor->SpecularOverride;

		// Get GBuffer
		const FSceneTextures& SceneTexturesData = ViewInfo.GetSceneTextures();
		TStaticArray<FTextureRenderTargetBinding, MaxSimultaneousRenderTargets> RenderTargetTextures;
		uint32 RenderTargetTextureCount = SceneTexturesData.GetGBufferRenderTargets(RenderTargetTextures);
		TArrayView<FTextureRenderTargetBinding> RenderTargetTexturesView(RenderTargetTextures.GetData(), RenderTargetTextureCount);

		FRDGTextureRef DepthTexture = GetIfProduced(SceneTexturesData.Depth.Target);
		check(DepthTexture);

		PassParameters->RenderTargets = GetRenderTargetBindings(ERenderTargetLoadAction::ELoad, RenderTargetTexturesView);
		PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(
			DepthTexture, ERenderTargetLoadAction::ELoad,
			ERenderTargetLoadAction::ENoAction, FExclusiveDepthStencil::DepthWrite_StencilNop 
		);

		FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(InView.FeatureLevel);

		TShaderMapRef<FScreenPassVS> VertexShader(ShaderMap);
		TShaderMapRef<FCameraFeedInjectionPS> PixelShader(ShaderMap);

		FScreenPassTextureViewport ViewPort(ViewInfo.ViewRect.Size());

		PassParameters->InputViewPort = GetScreenPassTextureViewportParameters(ViewPort);
		PassParameters->OutputViewPort = GetScreenPassTextureViewportParameters(ViewPort);

		using FCompositionDepthStencilState = TStaticDepthStencilState<true, CF_Greater>;

		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("StereolabsCameraFeedInjection"),
			InView,
			ViewPort,
			ViewPort,
			VertexShader,
			PixelShader,
			FCompositionDepthStencilState::GetRHI(),
			PassParameters
		);
	}
}
