#pragma once

#include "ScreenPass.h"


class FPreProcessDepthPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FPreProcessDepthPS)
	SHADER_USE_PARAMETER_STRUCT(FPreProcessDepthPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InViewPort)
		SHADER_PARAMETER_SAMPLER(SamplerState, sampler0)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, InTex)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};


class FRestrictPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FRestrictPS)
	SHADER_USE_PARAMETER_STRUCT(FRestrictPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InViewPort)
		SHADER_PARAMETER_SAMPLER(SamplerState, sampler0)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, InTex)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};


class FInterpolatePS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FInterpolatePS)
	SHADER_USE_PARAMETER_STRUCT(FInterpolatePS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InViewPort)
		SHADER_PARAMETER_SAMPLER(SamplerState, sampler0)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, InTex)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};


class FJacobiStepPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FJacobiStepPS)
	SHADER_USE_PARAMETER_STRUCT(FJacobiStepPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InViewPort)
		SHADER_PARAMETER_SAMPLER(SamplerState, sampler0)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, InTex)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};


// Post-processing on reconstructed depth, including clipping against specified planes
class FDepthClippingPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FDepthClippingPS)
	SHADER_USE_PARAMETER_STRUCT(FDepthClippingPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InViewPort)
		SHADER_PARAMETER_SAMPLER(SamplerState, sampler0)

		SHADER_PARAMETER(FMatrix44f, InvCameraProjectionMatrix)
		SHADER_PARAMETER(int32, bEnableFarClipping)
		SHADER_PARAMETER(float, FarClipDistance)
		SHADER_PARAMETER(int32, bEnableClippingPlane)
		SHADER_PARAMETER(FVector4f, UserClippingPlane)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, InTex)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};



class FVolumetricCompositionPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FVolumetricCompositionPS)
	SHADER_USE_PARAMETER_STRUCT(FVolumetricCompositionPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InViewPort)
		SHADER_PARAMETER_SAMPLER(SamplerState, sampler0)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, CameraColorTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, CameraDepthTexture) // Not RDG resource

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D, IntegratedLightScattering)
		SHADER_PARAMETER_SAMPLER(SamplerState, IntegratedLightScatteringSampler)

		SHADER_PARAMETER(float, VolumetricFogStartDistance)
		SHADER_PARAMETER(FVector3f, VolumetricFogInvGridSize)
		SHADER_PARAMETER(FVector3f, VolumetricFogGridZParams)
		SHADER_PARAMETER(FVector2f, VolumetricFogSVPosToVolumeUV)
		SHADER_PARAMETER(FVector2f, VolumetricFogUVMax)
		SHADER_PARAMETER(float, OneOverPreExposure)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};



class FRelightingPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FRelightingPS)
	SHADER_USE_PARAMETER_STRUCT(FRelightingPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, OutViewPort)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InViewPort)
		SHADER_PARAMETER_SAMPLER(SamplerState, sampler0)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, CameraColorTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, CameraDepthTexture) // Not RDG resource
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, CameraNormalTexture) // Not RDG resource

		SHADER_PARAMETER(FVector4f, LightColor)
		SHADER_PARAMETER(FVector3f, LightDirection)

		SHADER_PARAMETER(FMatrix44f, CameraLocalToWorld)
		SHADER_PARAMETER(FMatrix44f, CameraWorldToLocal)

		SHADER_PARAMETER(float, VirtualLightWeight)
		SHADER_PARAMETER(float, RealLightWeight)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};
