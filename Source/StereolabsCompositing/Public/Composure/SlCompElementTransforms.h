#pragma once

#include "CoreMinimal.h"

#include "CompositingElements/CompositingElementPasses.h"
#include "SlCompElementTransforms.generated.h"


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API UCompositingStereolabsDepthProcessingPass : public UCompositingElementTransform
{
	GENERATED_BODY()

public:

	// Reconstruction Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", InlineEditConditionToggle))
	bool bEnableJacobi = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnableJacobi"))
	int32 NumJacobiSteps = 10.0f;

	// Clipping Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", InlineEditConditionToggle))
	bool bEnableFarClipping = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnableFarClipping"))
	float FarClipDistance = 200.0f; // 200cm

	// Height of camera above floor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", InlineEditConditionToggle))
	bool bEnableFloorClipping = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnableFloorClipping"))
	float FloorClipDistance = 100.0f; // 100cm

public:
	virtual UTexture* ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera) override;

};


/**
 * Composites volumetric fog from the scene onto the camera image, using the real-world depth
 */
UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API UCompositingStereolabsVolumetricsPass : public UCompositingElementTransform
{		
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	FName DepthPassName;

	/** Used to get resources from the scene renderer that are required for composing volumetric effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositing Pass", meta = (DisplayAfter = "PassName", EditCondition = "bEnabled"))
	TWeakObjectPtr<class AStereolabsCompositingCaptureBase> StereolabsCGLayer;

public:
	virtual UTexture* ApplyTransform_Implementation(UTexture* Input, UComposurePostProcessingPassProxy* PostProcessProxy, ACameraActor* TargetCamera) override;

};
