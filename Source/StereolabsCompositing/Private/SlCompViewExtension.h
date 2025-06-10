#pragma once

#include "SceneViewExtension.h"


class AStereolabsCompositingCaptureBase;


class FSlCompViewExtension : public ISceneViewExtension
{
public:
	FSlCompViewExtension(AStereolabsCompositingCaptureBase* Owner);

	//~ Begin FSceneViewExtensionBase Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};

	virtual void PostRenderBasePassDeferred_RenderThread(
		FRDGBuilder& GraphBuilder,
		FSceneView& InView, 
		const FRenderTargetBindingSlots& RenderTargets, 
		TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override;
	virtual void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
	//~ End FSceneViewExtensionBase Interface

	// Check if this view extension is being invoked by the correct scene capture component
	// We do not want to apply any of the passes of this view extension to any other views
	bool IsRenderingToSlCaptureActor(const FSceneView& View) const;

private:

	void InjectCameraFeed(
		FRDGBuilder& GraphBuilder,
		FSceneView& InView,
		const FRenderTargetBindingSlots& RenderTargets,
		TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) const;
	// Implemented in SlCompVolumetricFogExtraction.cpp
	void ExtractVolumetricFog(FRDGBuilder& GraphBuilder, FSceneView& View) const;

private:
	TWeakObjectPtr<AStereolabsCompositingCaptureBase> CaptureActor;
};
