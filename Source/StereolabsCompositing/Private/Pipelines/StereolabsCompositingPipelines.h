#pragma once

#include "RenderGraphFwd.h"


struct DepthRelaxationParameters
{
	bool bEnableJacobiSteps;
	uint32 NumJacobiSteps;
};

namespace StereolabsCompositing
{
	void ExecuteDepthRelaxationPipeline(
		FRDGBuilder& GraphBuilder,
		const DepthRelaxationParameters& Parameters,
		FRDGTextureRef InTexture,				
		FRDGTextureRef OutTexture				
	);
}
