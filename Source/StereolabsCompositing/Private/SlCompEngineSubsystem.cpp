// Fill out your copyright notice in the Description page of Project Settings.

#include "SlCompEngineSubsystem.h"

#include "StereolabsCompositing.h"
#include "StereolabsCompositingSettings.h"

#include "Core/StereolabsCoreGlobals.h"


void LogCameraInitParams(const FSlInitParameters& InitParams)
{
	UE_LOG(LogStereolabsCompositing, Display, TEXT("Camera Init Params:"));

	FString InputType = UEnum::GetValueAsString(InitParams.InputType);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Input Type:	%s"), *InputType);

	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Serial:		%d"), InitParams.SerialNumber);

	FString Resolution = UEnum::GetValueAsString(InitParams.Resolution);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Resolution:	%s"), *Resolution);

	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t FPS:			%d"), InitParams.FPS);

	FString DepthMode = UEnum::GetValueAsString(InitParams.DepthMode);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Depth Mode:	%s"), *DepthMode);

	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Stabilization:	%d"), InitParams.DepthStabilization);
}

void LogCameraLensParameters(const FSlCameraParameters& Parameters, const FString& Label)
{
	UE_LOG(LogStereolabsCompositing, Display, TEXT("Camera Lens (%s) Params:"), *Label);

	const auto& Distortion = Parameters.Disto.IsEmpty() ? TArray<float>{5} : Parameters.Disto;
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Distortion:		[ %.2f, %.2f, %.2f, %.2f, %.2f ]"), 
		Distortion[0], Distortion[1], Distortion[2], Distortion[3], Distortion[4]);

	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Resolution:		(%d, %d)"), Parameters.Resolution.X, Parameters.Resolution.Y);

	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t HFOV:			%.2f"), Parameters.HFOV);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t VFOV:			%.2f"), Parameters.VFOV);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t HFocal:			%.2f"), Parameters.HFocal);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t VFocal:			%.2f"), Parameters.VFocal);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t OpticalCentreX:	%.2f"), Parameters.OpticalCenterX);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t OpticalCentreY:	%.2f"), Parameters.OpticalCenterY);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t FLMetric:		%.2f"), Parameters.FocalLengthMetric);
}

void LogCameraInformation(const FSlCameraInformation& CameraInfo)
{
	UE_LOG(LogStereolabsCompositing, Display, TEXT("Camera Information:"));

	FString Model = UEnum::GetValueAsString(CameraInfo.CameraModel);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Model:			%s"), *Model);
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Resolution:	(%d, %d)"), CameraInfo.Resolution.X, CameraInfo.Resolution.Y);

	const auto& Calibration = CameraInfo.CalibrationParameters;
	UE_LOG(LogStereolabsCompositing, Display, TEXT("Camera Extrinsic Calibration:"));
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Rotation:		%s"), *Calibration.Rotation.ToCompactString());
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Translation:	%s"), *Calibration.Translation.ToCompactString());
	LogCameraLensParameters(Calibration.LeftCameraParameters, TEXT("Left"));
	LogCameraLensParameters(Calibration.RightCameraParameters, TEXT("Right"));

	const auto& CalibrationRaw = CameraInfo.CalibrationParametersRaw;
	UE_LOG(LogStereolabsCompositing, Display, TEXT("Camera (Unrectified) Extrinsic Calibration:"));
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Rotation:		%s"), *CalibrationRaw.Rotation.ToCompactString());
	UE_LOG(LogStereolabsCompositing, Display, TEXT("	\t Translation:	%s"), *CalibrationRaw.Translation.ToCompactString());
	LogCameraLensParameters(CalibrationRaw.LeftCameraParameters, TEXT("Left - Unrectified"));
	LogCameraLensParameters(CalibrationRaw.RightCameraParameters, TEXT("Right  - Unrectified"));
}


ESlTextureFormat GetTextureFormatForView(ESlView View)
{
	switch (View)
	{
		case ESlView::V_Left:				return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_Right:				return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_LeftUnrectified:	return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_RightUnrectified:	return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_SideBySide:			return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_Depth:				return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_Confidence:			return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_Normals:			return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_DepthRight:			return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		case ESlView::V_NormalsRight:		return ESlTextureFormat::TF_R8G8B8A8_SNORM;
		default:
		{
			checkNoEntry();
			return ESlTextureFormat::TF_Unkown;
		}
	}
}

ESlTextureFormat GetTextureFormatForMeasure(ESlMeasure Measure)
{
	switch (Measure)
	{
		case ESlMeasure::M_Disparity:			return ESlTextureFormat::TF_R32_FLOAT;
		case ESlMeasure::M_Depth:				return ESlTextureFormat::TF_R32_FLOAT;
		case ESlMeasure::M_Confidence:			return ESlTextureFormat::TF_R32_FLOAT;
		case ESlMeasure::M_XYZ:					return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_RGBA:			return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_BGRA:			return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_ARGB:			return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_ABGR:			return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_Normals:				return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_DisparityRight:		return ESlTextureFormat::TF_R32_FLOAT;
		case ESlMeasure::M_DepthRight:			return ESlTextureFormat::TF_R32_FLOAT;
		case ESlMeasure::M_XYZ_Right:			return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_RGBA_Right:		return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_BGRA_Right:		return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_ARGB_Right:		return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_XYZ_ABGR_Right:		return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_NormalsRight:		return ESlTextureFormat::TF_A32B32G32R32F;
		case ESlMeasure::M_DEPTH_U16_MM:		return ESlTextureFormat::TF_R32_FLOAT;
		case ESlMeasure::M_DEPTH_U16_MM_RIGHT:	return ESlTextureFormat::TF_R32_FLOAT;
		default:
		{
			checkNoEntry();
			return ESlTextureFormat::TF_Unkown;
		}
	}
}


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
			LogCameraInitParams(InitParams);

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

	(void)Batch->Tick();
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
	// Passing FIntPoint::ZeroValue gets information for current resolution
	FSlCameraInformation CameraInformation = GSlCameraProxy->GetCameraInformation(FIntPoint::ZeroValue);
	LogCameraInformation(CameraInformation);

	// Create textures and texture batch
	Batch.Reset(USlGPUTextureBatch::CreateGPUTextureBatch(FName("CameraBatch")));
	for (const auto& WrapperPtr : Wrappers)
	{
		if (auto Wrapper = WrapperPtr.Pin())
		{
			Wrapper->CreateTexture(FSlCompImageWrapper::PassKey{}, Batch.Get());
		}
	}

	// Calculate camera parameters
	FSlCameraParameters CameraParameters = GSlCameraProxy->CameraInformation.CalibrationParameters.LeftCameraParameters;
	HorizontalFieldOfView = FMath::DegreesToRadians(CameraParameters.HFOV);
	VerticalFieldOfView = FMath::DegreesToRadians(CameraParameters.VFOV);

	// Calculate a projection matrix based off of the camera properties
	const float HalfFovX = 0.5f * HorizontalFieldOfView;
	const float HalfFovY = 0.5f * VerticalFieldOfView;
	CameraProjectionMatrix = FMatrix(
		FPlane(1.0f / FMath::Tan(HalfFovX), 0.0f,							0.0f, 0.0f),
		FPlane(0.0f,							1.0f / FMath::Tan(HalfFovY),		0.0f, 0.0f),
		FPlane(0.0f,							0.0f,							0.0f, 1.0f),
		FPlane(0.0f,							0.0f,							0.1f, 0.0f)
	);
	CameraInvProjectionMatrix = CameraProjectionMatrix.Inverse();
}

void USlCompEngineSubsystem::OnCameraClosed()
{
	for (const auto& WrapperPtr : Wrappers)
	{
		if (auto Wrapper = WrapperPtr.Pin())
		{
			Wrapper->DestroyTexture(FSlCompImageWrapper::PassKey{});
		}
	}

	// Empty and release the batch
	Batch->Clear();
	Batch.Reset();
}

const FMatrix& USlCompEngineSubsystem::GetProjectionMatrix()
{
	return CameraProjectionMatrix;
}

const FMatrix& USlCompEngineSubsystem::GetInvProjectionMatrix()
{
	return CameraInvProjectionMatrix;
}

TSharedPtr<FSlCompImageWrapper> USlCompEngineSubsystem::GetOrCreateImageWrapperImpl(FSlCompImageWrapperTarget&& Target)
{
	// Try to find an existing wrapper over this target
	for (const auto& WrapperPtr : Wrappers)
	{
		if (WrapperPtr.IsValid())
		{
			auto Wrapper = WrapperPtr.Pin();
			if (Wrapper->Matches(Target))
			{
				return Wrapper;
			}
		}
	}
	
	// First perform a quick compaction on the wrappers list removing any dead pointers
	Wrappers.RemoveAll([](const auto& Ptr){ return !Ptr.IsValid(); });

	// No wrapper over this target exists yet
	auto Wrapper = MakeShared<FSlCompImageWrapper>(
		FSlCompImageWrapper::PassKey{},
		std::move(Target),
		Batch.Get());

	Wrappers.Emplace(Wrapper);

	return Wrapper;
}


FSlCompImageWrapper::FSlCompImageWrapper(const PassKey&, FSlCompImageWrapperTarget&& InTarget, TObjectPtr<USlTextureBatch> InBatch)
	: Target(std::move(InTarget))
{
	CreateTexture(PassKey{}, InBatch);
}

FSlCompImageWrapper::~FSlCompImageWrapper()
{
	DestroyTexture(PassKey{});
}

void FSlCompImageWrapper::CreateTexture(const PassKey&, TObjectPtr<USlTextureBatch> InBatch)
{
	TextureBatch.Reset(InBatch);

	if (TextureBatch)
	{
		// TextureBatch should only be created when the camera is open
		check(GSlCameraProxy->IsCameraOpened());

		// Get resolution (passing FIntPoint::ZeroValue gets current camera information)
		FSlCameraInformation CameraInformation = GSlCameraProxy->GetCameraInformation(FIntPoint::ZeroValue);
		const FIntPoint& Resolution = CameraInformation.Resolution;

		// Create texture
		if (Target.IsType<ESlView>())
		{
			ESlView View = Target.Get<ESlView>();
			FName ViewName{ UEnum::GetValueAsString(View) };

			Texture.Reset(
				USlViewTexture::CreateGPUViewTexture(ViewName, Resolution.X, Resolution.Y, View, true, GetTextureFormatForView(View))
			);
		}
		else if (Target.IsType<ESlMeasure>())
		{
			ESlMeasure Measure = Target.Get<ESlMeasure>();
			FName MeasureName{ UEnum::GetValueAsString(Measure) };

			Texture.Reset(
				USlMeasureTexture::CreateGPUMeasureTexture(MeasureName, Resolution.X, Resolution.Y, Measure, true, GetTextureFormatForMeasure(Measure))
			);
		}
		else
		{
			checkNoEntry();
		}

		TextureBatch->AddTexture(Texture.Get());
	}
}

void FSlCompImageWrapper::DestroyTexture(const PassKey&)
{
	if (TextureBatch && Texture)
	{
		TextureBatch->RemoveTexture(Texture.Get());
	}
	Texture.Reset();
	TextureBatch.Reset();
}

UTexture2D* FSlCompImageWrapper::GetTexture() const
{
	return Texture ? Texture->Texture : nullptr;
}

bool FSlCompImageWrapper::Matches(const FSlCompImageWrapperTarget& InTarget)
{
	if (InTarget.GetIndex() != Target.GetIndex())
	{
		return false;
	}

	// InTarget and Target have the same index
	if (InTarget.IsType<ESlView>())
	{
		return InTarget.Get<ESlView>() == Target.Get<ESlView>();
	}
	if(InTarget.IsType<ESlMeasure>())
	{
		return InTarget.Get<ESlMeasure>() == Target.Get<ESlMeasure>();
	}

	checkNoEntry();
	return false;
}
