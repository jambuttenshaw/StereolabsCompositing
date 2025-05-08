// Fill out your copyright notice in the Description page of Project Settings.

#include "StereolabsCompositingEngineSubsystem.h"

#include "StereolabsCompositing.h"

#include "Core/StereolabsCoreGlobals.h"
#include "Core/StereolabsCameraProxy.h"
#include "StereolabsCompositingSettings.h"
#include "Core/StereolabsTextureBatch.h"


void UStereolabsCompositingEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	bCanEverTick = true;

	CreateSlCameraProxyInstance();

	if (GSlCameraProxy)
	{
		// Hook delegates
		GSlCameraProxy->OnCameraOpened.AddDynamic(this, &UStereolabsCompositingEngineSubsystem::OnCameraOpened);
		GSlCameraProxy->OnCameraClosed.AddDynamic(this, &UStereolabsCompositingEngineSubsystem::OnCameraClosed);

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

void UStereolabsCompositingEngineSubsystem::Deinitialize()
{
	if (GSlCameraProxy->IsCameraOpened())
	{
		GSlCameraProxy->CloseCamera();
	}

	FreeSlCameraProxyInstance();
}


void UStereolabsCompositingEngineSubsystem::Tick(float DeltaTime)
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

bool UStereolabsCompositingEngineSubsystem::IsTickable() const
{
	return bCanEverTick && GSlCameraProxy->IsCameraOpened();
}

TStatId UStereolabsCompositingEngineSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UStereolabsCompositingEngineSubsystem, STATGROUP_Tickables);
}


void UStereolabsCompositingEngineSubsystem::OnCameraOpened()
{
	// Create textures and texture batch
	Batch = USlGPUTextureBatch::CreateGPUTextureBatch(FName("CameraBatch"));

	FIntPoint Resolution = GSlCameraProxy->CameraInformation.CalibrationParameters.LeftCameraParameters.Resolution;

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
}

void UStereolabsCompositingEngineSubsystem::OnCameraClosed()
{
	// Do not explicitly release Batch and Textures here - this causes double-free somehow
}


UTexture2D* UStereolabsCompositingEngineSubsystem::GetColorTexture()
{
	return ColorTexture ? ColorTexture->Texture : nullptr;
}

UTexture2D* UStereolabsCompositingEngineSubsystem::GetDepthTexture()
{
	return DepthTexture ? DepthTexture->Texture : nullptr;
}

UTexture2D* UStereolabsCompositingEngineSubsystem::GetNormalTexture()
{
	return NormalTexture ? NormalTexture->Texture : nullptr;
}
