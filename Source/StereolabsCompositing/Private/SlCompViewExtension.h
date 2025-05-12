#pragma once

#include "SceneViewExtension.h"


class AStereolabsCompositingCaptureBase;


class FSlCompViewExtension : public FSceneViewExtensionBase
{
public:
	FSlCompViewExtension(const FAutoRegister& AutoRegister, AStereolabsCompositingCaptureBase* Owner);

	//~ Begin FSceneViewExtensionBase Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};

	virtual void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
	//~ End FSceneViewExtensionBase Interface

private:
	TWeakObjectPtr<AStereolabsCompositingCaptureBase> CaptureActor;
};
