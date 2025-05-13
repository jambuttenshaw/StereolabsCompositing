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


// Resources and parameters extracted from the scene render graph to be able to apply volumetric fog in composure
struct FVolumetricFogRequiredData
{
	// Resources
	TRefCountPtr<IPooledRenderTarget> IntegratedLightScatteringTexture;

	// Associated parameters
	float VolumetricFogStartDistance;
	FVector3f VolumetricFogInvGridSize;
	FVector3f VolumetricFogGridZParams;
	FVector2f VolumetricFogSVPosToVolumeUV;
	FVector2f VolumetricFogUVMax;
	float OneOverPreExposure;

	bool IsValid() const
	{
		bool bValid = true;
		bValid &= IntegratedLightScatteringTexture.IsValid();
		return bValid;
	}
};

struct FVolumetricsCompositionParametersProxy
{
	UTexture* CameraDepthTexture;

	const FVolumetricFogRequiredData* VolumetricFogData;

	bool IsValid() const
	{
		bool bValid = true;
		bValid &= CameraDepthTexture != nullptr;
		bValid &= VolumetricFogData != nullptr;
		bValid &= VolumetricFogData->IsValid();
		return bValid;
	}
};


namespace StereolabsCompositing
{
	void ExecuteDepthProcessingPipeline(
		FRDGBuilder& GraphBuilder,
		const FDepthProcessingParametersProxy& Parameters,
		FRDGTextureRef InTexture,				
		FRDGTextureRef OutTexture				
	);

	void ExecuteVolumetricsCompositionPipeline(
		FRDGBuilder& GraphBuilder,
		const FVolumetricsCompositionParametersProxy& Parameters,
		FRDGTextureRef InTexture,
		FRDGTextureRef OutTexture
	);
}
