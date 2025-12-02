// Fill out your copyright notice in the Description page of Project Settings.

#include "SlCompEngineSubsystem.h"

#include "StereolabsCompositing.h"
#include "StereolabsCompositingSettings.h"

#include "Core/StereolabsCoreGlobals.h"

#include "CanvasTypes.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"


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


FSlCompImageTarget::FSlCompImageTarget(ESlView InView, bool bInInverseTonemapping)
	: ViewOrMeasure(TInPlaceType<ESlView>(), InView)
	, bInverseTonemapping(bInInverseTonemapping)
{
}

FSlCompImageTarget::FSlCompImageTarget(ESlMeasure InMeasure)
	: ViewOrMeasure(TInPlaceType<ESlMeasure>(), InMeasure)
{
}

TOptional<ESlView> FSlCompImageTarget::GetView() const
{
	if (ViewOrMeasure.IsType<ESlView>())
	{
		return ViewOrMeasure.Get<ESlView>();
	}
	return NullOpt;
}

TOptional<ESlMeasure> FSlCompImageTarget::GetMeasure() const
{
	if (ViewOrMeasure.IsType<ESlMeasure>())
	{
		return ViewOrMeasure.Get<ESlMeasure>();
	}
	return NullOpt;
}

bool FSlCompImageTarget::operator==(const FSlCompImageTarget& Other) const
{
	if (Other.ViewOrMeasure.GetIndex() != ViewOrMeasure.GetIndex())
	{
		return false;
	}

	// InTarget and Target have the same index
	if (Other.ViewOrMeasure.IsType<ESlView>())
	{
		if (Other.ViewOrMeasure.Get<ESlView>() != ViewOrMeasure.Get<ESlView>())
			return false;
	}
	if (Other.ViewOrMeasure.IsType<ESlMeasure>())
	{
		if (Other.ViewOrMeasure.Get<ESlMeasure>() != ViewOrMeasure.Get<ESlMeasure>())
			return false;
	}

	if (Other.bInverseTonemapping != bInverseTonemapping)
	{
		return false;
	}

	return true;
}


USlCompEngineSubsystem::USlCompEngineSubsystem()
{
	static const FString InvTonemappingMatRef = "/Script/Engine.Material'/StereolabsCompositing/StereolabsCompositing/Materials/M_StereolabsInverseTonemapping.M_StereolabsInverseTonemapping'";
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> InvTonemappingMatFinder(*InvTonemappingMatRef);

	if (InvTonemappingMatFinder.Succeeded())
	{
		InverseTonemappingMID = UMaterialInstanceDynamic::Create(Cast<UMaterialInterface>(InvTonemappingMatFinder.Object), this, "MID_StereolabsInverseTonemapping");
	}
	else
	{
		UE_LOG(LogStereolabsCompositing, Error, TEXT("Failed to find inverse tonemapping material!"));
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

	if (Batch->Tick())
	{
		// Some textures want to respond to new textures being acquired
		for (auto WrapperPtr : Wrappers)
		{
			if (auto Wrapper = WrapperPtr.Pin())
			{
				Wrapper->OnTextureUpdated(ISlCompImageWrapper::FPassKey{});
			}
		}
	}
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
			Wrapper->CreateTexture(FSlCompImageWrapper::FPassKey{}, Batch.Get());
		}
	}

	// Calculate camera parameters
	FSlCameraParameters CameraParameters = GSlCameraProxy->CameraInformation.CalibrationParameters.LeftCameraParameters;
	HorizontalFieldOfView = FMath::DegreesToRadians(CameraParameters.HFOV);
	VerticalFieldOfView = FMath::DegreesToRadians(CameraParameters.VFOV);

	// Calculate a projection matrix based off of the camera properties
	// Introduction of a near plane (0.1cm) is required to make the matrix invertible
	// The choice of near plane doesn't matter - as we don't ever care about the depth value after projection,
	// and we use the data sampled from the depth texture to un-project anyway.
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
			Wrapper->DestroyTexture(FSlCompImageWrapper::FPassKey{});
		}
	}

	// Empty and release the batch
	Batch->Clear();
	Batch.Reset();
}

TSharedPtr<ISlCompImageWrapper> USlCompEngineSubsystem::GetOrCreateImageWrapperImpl(FSlCompImageTarget&& Target)
{
	// Try to find an existing wrapper over this target
	for (const auto& WrapperPtr : Wrappers)
	{
		if (WrapperPtr.IsValid())
		{
			auto Wrapper = WrapperPtr.Pin();
			if (Wrapper->MatchesTarget(Target))
			{
				return Wrapper;
			}
		}
	}
	
	// First perform a quick compaction on the wrappers list removing any dead pointers
	Wrappers.RemoveAll([](const auto& Ptr){ return !Ptr.IsValid(); });

	// No wrapper over this target exists yet
	TSharedPtr<ISlCompImageWrapper> Wrapper;

	if (Target.IsInverseTonemappingEnabled())
	{
		Wrapper = MakeShared<FSlCompTonemappedImageWrapper>(
			FSlCompTonemappedImageWrapper::FPassKey{},
			std::move(Target));
	}
	else
	{
		Wrapper = MakeShared<FSlCompImageWrapper>(
			FSlCompImageWrapper::FPassKey{},
			std::move(Target));
	}
	Wrapper->CreateTexture(FSlCompImageWrapper::FPassKey{}, Batch.Get());

	Wrappers.Emplace(Wrapper);

	return Wrapper;
}

void USlCompEngineSubsystem::DoInverseTonemapping(UTexture* Input, UTextureRenderTarget2D* Output)
{
	if (!FApp::CanEverRender())
	{
		// Returning early to avoid warnings about missing resources that are expected when CanEverRender is false.
		return;
	}

	check(InverseTonemappingMID && "Material instance dynamic should exist!");
	check(Output && Output->GetResource() && "Output should exist!");
	check(Input && Input->GetResource() && "Input should exist!");

	InverseTonemappingMID->SetTextureParameterValue("Color", Input);

	// This is a user-facing function, so we'd rather make sure that shaders are ready by the time we render, in order to ensure we don't draw with a fallback material
	InverseTonemappingMID->EnsureIsComplete();
	FTextureRenderTargetResource* RenderTargetResource = Output->GameThread_GetRenderTargetResource();

	FCanvas RenderCanvas(
		RenderTargetResource,
		nullptr,
		FGameTime(),
		GMaxRHIFeatureLevel);

	if (!Canvas)
	{
		Canvas = NewObject<UCanvas>(GetTransientPackage(), NAME_None);
	}
	Canvas->Init(Output->SizeX, Output->SizeY, nullptr, &RenderCanvas);

	{
		RHI_BREADCRUMB_EVENT_GAMETHREAD_F("DrawMaterialToRenderTarget", "DrawMaterialToRenderTarget: %s", Output->GetFName());

		ENQUEUE_RENDER_COMMAND(FlushDeferredResourceUpdateCommand)(
			[RenderTargetResource](FRHICommandListImmediate& RHICmdList)
			{
				RenderTargetResource->FlushDeferredResourceUpdate(RHICmdList);
			});

		Canvas->K2_DrawMaterial(InverseTonemappingMID, FVector2D(0, 0), FVector2D(Output->SizeX, Output->SizeY), FVector2D(0, 0));

		RenderCanvas.Flush_GameThread();
		Canvas->Canvas = nullptr;

		//UpdateResourceImmediate must be called here to ensure mips are generated.
		Output->UpdateResourceImmediate(false);
	}
}


FSlCompImageWrapper::FSlCompImageWrapper(const FPassKey&, FSlCompImageTarget&& InTarget)
	: Target(std::move(InTarget))
{
}

FSlCompImageWrapper::~FSlCompImageWrapper()
{
	if (TextureBatch && Texture)
	{
		TextureBatch->RemoveTexture(Texture.Get());
	}
}

void FSlCompImageWrapper::CreateTexture(const FPassKey&, TObjectPtr<USlTextureBatch> InBatch)
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
		if (auto View = Target.GetView())
		{
			FName ViewName{ UEnum::GetValueAsString(*View) };

			Texture.Reset(
				USlViewTexture::CreateGPUViewTexture(ViewName, Resolution.X, Resolution.Y, *View, true, GetTextureFormatForView(*View))
			);
		}
		else if (auto Measure = Target.GetMeasure())
		{
			FName MeasureName{ UEnum::GetValueAsString(*Measure) };

			Texture.Reset(
				USlMeasureTexture::CreateGPUMeasureTexture(MeasureName, Resolution.X, Resolution.Y, *Measure, true, GetTextureFormatForMeasure(*Measure))
			);
		}
		else
		{
			checkNoEntry();
		}

		TextureBatch->AddTexture(Texture.Get());
	}
}

void FSlCompImageWrapper::DestroyTexture(const FPassKey&)
{
	if (TextureBatch && Texture)
	{
		TextureBatch->RemoveTexture(Texture.Get());
	}
	Texture.Reset();
	TextureBatch.Reset();
}

UTexture* FSlCompImageWrapper::GetTexture() const
{
	return Texture ? Texture->Texture : nullptr;
}

bool FSlCompImageWrapper::MatchesTarget(const FSlCompImageTarget& InTarget) const
{
	return Target == InTarget;
}


FSlCompTonemappedImageWrapper::FSlCompTonemappedImageWrapper(const FPassKey&, FSlCompImageTarget&& InTarget)
	: Target(std::move(InTarget))
{
	checkNoRecursion();

	auto Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>();
	check(Subsystem); // Should exist if we managed to end up inside this constructor

	// Get or create wrapper without tonemapping to serve us the underlying image
	if (auto View = InTarget.GetView())
	{
		ImageWrapper = Subsystem->GetOrCreateImageWrapper(*View);
	}
	else if (auto Measure = InTarget.GetMeasure())
	{
		ImageWrapper = Subsystem->GetOrCreateImageWrapper(*Measure);
	}
	else
	{
		checkNoEntry();
	}
}

void FSlCompTonemappedImageWrapper::CreateTexture(const FPassKey&, TObjectPtr<USlTextureBatch> InBatch)
{
	// If there is no batch, then the underlying image wrapper will not have a texture either
	if (!InBatch)
	{
		return;
	}

	// If batch is valid then source texture should definitely exist
	UTexture* SourceTexture = ImageWrapper->GetTexture();
	check(SourceTexture);
	UTexture2D* SourceTexture2D = Cast<UTexture2D>(SourceTexture);
	check(SourceTexture2D);

	TonemappedTexture.Reset(NewObject<UTextureRenderTarget2D>(GetTransientPackage()));
	check(TonemappedTexture);

	EPixelFormat Format = SourceTexture2D->GetPixelFormat();
	if (!FTextureRenderTargetResource::IsSupportedFormat(Format))
	{
		Format = PF_R8G8B8A8;
	}

	TonemappedTexture->ClearColor = FLinearColor::Black;
	TonemappedTexture->bAutoGenerateMips = false;
	TonemappedTexture->bCanCreateUAV = false;
	TonemappedTexture->InitCustomFormat(SourceTexture2D->GetSizeX(), SourceTexture2D->GetSizeY(), Format, true);
	TonemappedTexture->UpdateResourceImmediate(true);
}

void FSlCompTonemappedImageWrapper::DestroyTexture(const FPassKey&)
{
	TonemappedTexture.Reset();
}

void FSlCompTonemappedImageWrapper::OnTextureUpdated(const FPassKey&)
{
	// Perform tonemapping pass on image from underlying wrapper
	if (!TonemappedTexture)
		return;

	if (auto Source = ImageWrapper->GetTexture())
	{
		auto Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>();
		check(Subsystem);

		Subsystem->DoInverseTonemapping(Source, TonemappedTexture.Get());
	}
}

UTexture* FSlCompTonemappedImageWrapper::GetTexture() const
{
	return TonemappedTexture ? TonemappedTexture.Get() : nullptr;
}

bool FSlCompTonemappedImageWrapper::MatchesTarget(const FSlCompImageTarget& InTarget) const
{
	return Target == InTarget;
}
