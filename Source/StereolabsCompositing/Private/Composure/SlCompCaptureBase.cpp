#include "Composure/SlCompCaptureBase.h"

#include "SceneViewExtension.h"
#include "SlCompViewExtension.h"

#include "Pipelines/StereolabsCompositingPipelines.h"


AStereolabsCompositingCaptureBase::AStereolabsCompositingCaptureBase()
{
	PrimaryActorTick.bCanEverTick = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SlCompViewExtension = FSceneViewExtensions::NewExtension<FSlCompViewExtension>(this);
		VolumetricFogData_RenderThread = MakeUnique<FVolumetricFogRequiredData>();
	}
}
