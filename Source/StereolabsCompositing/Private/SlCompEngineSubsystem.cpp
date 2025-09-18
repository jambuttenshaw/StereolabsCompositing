// Fill out your copyright notice in the Description page of Project Settings.

#include "SlCompEngineSubsystem.h"

#include "StereolabsCompositing.h"

#include "Core/StereolabsCoreGlobals.h"
#include "Core/StereolabsCameraProxy.h"
#include "StereolabsCompositingSettings.h"
#include "Core/StereolabsTextureBatch.h"


void USlCompEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	bCanEverTick = true;

	CreateSlCameraProxyInstance();

	if (GSlCameraProxy)
	{
		// Hook delegates
		GSlCameraProxy->OnCameraOpened.AddDynamic(this, &USlCompEngineSubsystem::OnCameraOpened);
		GSlCameraProxy->OnCameraClosed.AddDynamic(this, &USlCompEngineSubsystem::OnCameraClosed);

		if(!GSlCameraProxy->IsCameraOpened())
		{
			const FSlInitParameters& InitParams = GetMutableDefault<UStereolabsCompositingSettings>()->InitParams;

			if (InitParams.bLoop)
			{
				GSlCameraProxy->SetSVOPlaybackLooping(true);
			}

			auto runtime = FSlRuntimeParameters();
			runtime.bEnableDepth = true;

			auto settings = FSlVideoSettings();
			GSlCameraProxy->SetCameraSettings(settings);
			GSlCameraProxy->SetRuntimeParameters(runtime);

			GSlCameraProxy->OpenCamera(InitParams);

			GSlCameraProxy->EnableGrabThread(true);
		}
	}
};

void USlCompEngineSubsystem::Deinitialize()
{
	if (GSlCameraProxy->IsCameraOpened())
	{
		GSlCameraProxy->CloseCamera();
	}

	FreeSlCameraProxyInstance();
}


void USlCompEngineSubsystem::Tick(float DeltaTime)
{
	if (!GSlCameraProxy->IsCameraOpened())
	{
		UE_LOG(LogStereolabsCompositing, Error, TEXT("Stereolabs Compositing Subsystem should not be ticking while camera is not open!"));
		return;
	}

	if (!Batch)
	{
		return;
	}

	bool bNewImage = Batch->Tick();
}

bool USlCompEngineSubsystem::IsTickable() const
{
	return bCanEverTick && GSlCameraProxy->IsCameraOpened();
}

TStatId USlCompEngineSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USlCompEngineSubsystem, STATGROUP_Tickables);
}


void USlCompEngineSubsystem::OnCameraOpened()
{
	// Create textures and texture batch
	Batch = USlGPUTextureBatch::CreateGPUTextureBatch(FName("CameraBatch"));

	FSlCameraParameters CameraParameters = GSlCameraProxy->CameraInformation.CalibrationParameters.LeftCameraParameters;
	FIntPoint Resolution = CameraParameters.Resolution;

	ColorTexture = USlViewTexture::CreateGPUViewTexture(
		"ColorTexture", Resolution.X, Resolution.Y, ESlView::V_Left, true, ESlTextureFormat::TF_R8G8B8A8_SNORM
	);
	Batch->AddTexture(ColorTexture);

	DepthTexture = USlMeasureTexture::CreateGPUMeasureTexture(
		"DepthTexture", Resolution.X, Resolution.Y, ESlMeasure::M_Depth, true, ESlTextureFormat::TF_R32_FLOAT
	);
	Batch->AddTexture(DepthTexture);

	NormalTexture = USlMeasureTexture::CreateGPUMeasureTexture(
		"NormalTexture", Resolution.X, Resolution.Y, ESlMeasure::M_Normals, true, ESlTextureFormat::TF_A32B32G32R32F
	);
	Batch->AddTexture(NormalTexture);

	HorizontalFieldOfView = FMath::DegreesToRadians(CameraParameters.HFOV);
	VerticalFieldOfView = FMath::DegreesToRadians(CameraParameters.VFOV);

	// Calculate a projection matrix based off of the camera properties
	// Setting min == max will project far plane to infinity
	// In practice camera cannot calculate depth at distances <~30cm, so setting near plane to any value below this is adequate
	const float HalfFovX = 0.5f * HorizontalFieldOfView;
	const float HalfFovY = 0.5f * VerticalFieldOfView;
	CameraProjectionMatrix = FReversedZPerspectiveMatrix(HalfFovX, HalfFovY, 1.0f, 1.0f, NearClippingPlane, NearClippingPlane);
	CameraInvProjectionMatrix = CameraProjectionMatrix.Inverse();
}

void USlCompEngineSubsystem::OnCameraClosed()
{
	// Do not explicitly release Batch and Textures here - this causes double-free
}


UTexture2D* USlCompEngineSubsystem::GetColorTexture()
{
	return ColorTexture ? ColorTexture->Texture : nullptr;
}

UTexture2D* USlCompEngineSubsystem::GetDepthTexture()
{
	return DepthTexture ? DepthTexture->Texture : nullptr;
}

UTexture2D* USlCompEngineSubsystem::GetNormalTexture()
{
	return NormalTexture ? NormalTexture->Texture : nullptr;
}

const FMatrix& USlCompEngineSubsystem::GetProjectionMatrix()
{
	return CameraProjectionMatrix;
}

const FMatrix& USlCompEngineSubsystem::GetInvProjectionMatrix()
{
	return CameraInvProjectionMatrix;
}
