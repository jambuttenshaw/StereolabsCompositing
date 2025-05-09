#pragma once

#include "RenderGraphFwd.h"


struct FDepthProcessingParametersProxy
{
	// Relaxation parameters
	bool bEnableJacobiSteps;
	uint32 NumJacobiSteps;

	// Post-processing parameters
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
