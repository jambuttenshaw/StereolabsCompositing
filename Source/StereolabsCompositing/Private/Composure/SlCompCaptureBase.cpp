#include "Composure/SlCompCaptureBase.h"

#include "SceneViewExtension.h"
#include "SlCompViewExtension.h"

#include "Pipelines/SlCompPipelines.h"


AStereolabsCompositingCaptureBase::AStereolabsCompositingCaptureBase()
{
	PrimaryActorTick.bCanEverTick = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SlCompViewExtension = FSceneViewExtensions::NewExtension<FSlCompViewExtension>(this);
		VolumetricFogData_RenderThread = MakeShared<FVolumetricFogRequiredData>();
	}
}

const FVolumetricFogRequiredData* AStereolabsCompositingCaptureBase::GetVolumetricFogData() const
{
	return VolumetricFogData_RenderThread.Get();
}

FVolumetricFogRequiredData* AStereolabsCompositingCaptureBase::GetVolumetricFogData()
{
	return VolumetricFogData_RenderThread.Get();
}

FStereolabsCameraTextures AStereolabsCompositingCaptureBase::GetCameraTextures()
{
	FStereolabsCameraTextures OutTextures;
	OutTextures.ColorTexture = FindNamedRenderResult(CameraColorPassName);
	OutTextures.DepthTexture = FindNamedRenderResult(CameraDepthPassName);
	OutTextures.NormalsTexture = FindNamedRenderResult(CameraNormalsPassName);
	return OutTextures;
}
