#pragma once

#include "RenderGraphFwd.h"


struct FDepthProcessingParametersProxy
{
	// Camera properties
	FMatrix44f InvProjectionMatrix; // for projecting points into view space

	// Relaxation parameters
	bool bEnableJacobiSteps;
	uint32 NumJacobiSteps;

	// Post-processing parameters
	FVector4f UserClippingPlane;
	float FarClipDistance;
};

namespace StereolabsCompositing
{
	void ExecuteDepthProcessingPipeline(
		FRDGBuilder& GraphBuilder,
		const FDepthProcessingParametersProxy& Parameters,
		FRDGTextureRef InTexture,				
		FRDGTextureRef OutTexture				
	);
}
