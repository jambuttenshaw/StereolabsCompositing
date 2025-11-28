#include "ReprojectionCalibrationStereolabsTarget.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "CanvasTypes.h"

#include "StereolabsCompositing.h"

#define LOCTEXT_NAMESPACE "StereolabsCompositing"

// Copied from Composure\Private\CompositingElements\CompositingElementPasses.cpp
static bool GetTargetFormatFromPixelFormat(const EPixelFormat PixelFormat, ETextureRenderTargetFormat& OutRTFormat)
{
	switch (PixelFormat)
	{
	case PF_G8: OutRTFormat = RTF_R8; return true;
	case PF_R8G8: OutRTFormat = RTF_RG8; return true;
	case PF_B8G8R8A8: OutRTFormat = RTF_RGBA8; return true;

	case PF_R16F: OutRTFormat = RTF_R16f; return true;
	case PF_G16R16F: OutRTFormat = RTF_RG16f; return true;
	case PF_FloatRGBA: OutRTFormat = RTF_RGBA16f; return true;

	case PF_R32_FLOAT: OutRTFormat = RTF_R32f; return true;
	case PF_G32R32F: OutRTFormat = RTF_RG32f; return true;
	case PF_A32B32G32R32F: OutRTFormat = RTF_RGBA32f; return true;
	case PF_A2B10G10R10: OutRTFormat = RTF_RGB10A2; return true;
	default:
		break;
	}
	return false;
}


static void DrawMaterialToRenderTarget(UCanvas* Canvas, UTextureRenderTarget2D* TextureRenderTarget, UMaterialInterface* Material)
{
	if (!FApp::CanEverRender())
	{
		// Returning early to avoid warnings about missing resources that are expected when CanEverRender is false.
		return;
	}

	if (!Material)
	{
		FMessageLog("Blueprint").Warning(LOCTEXT("DrawMaterialToRenderTarget_InvalidMaterial", "DrawMaterialToRenderTarget: Material must be non-null."));
	}
	else if (!TextureRenderTarget)
	{
		FMessageLog("Blueprint").Warning(LOCTEXT("DrawMaterialToRenderTarget_InvalidTextureRenderTarget", "DrawMaterialToRenderTarget: TextureRenderTarget must be non-null."));
	}
	else if (!TextureRenderTarget->GetResource())
	{
		FMessageLog("Blueprint").Warning(LOCTEXT("DrawMaterialToRenderTarget_ReleasedTextureRenderTarget", "DrawMaterialToRenderTarget: render target has been released."));
	}
	else
	{
		// This is a user-facing function, so we'd rather make sure that shaders are ready by the time we render, in order to ensure we don't draw with a fallback material :
		Material->EnsureIsComplete();

		FTextureRenderTargetResource* RenderTargetResource = TextureRenderTarget->GameThread_GetRenderTargetResource();

		FCanvas RenderCanvas(
			RenderTargetResource,
			nullptr,
			FGameTime(),
			GMaxRHIFeatureLevel);

		Canvas->Init(TextureRenderTarget->SizeX, TextureRenderTarget->SizeY, nullptr, &RenderCanvas);

		{
			RHI_BREADCRUMB_EVENT_GAMETHREAD_F("DrawMaterialToRenderTarget", "DrawMaterialToRenderTarget: %s", TextureRenderTarget->GetFName());

			ENQUEUE_RENDER_COMMAND(FlushDeferredResourceUpdateCommand)(
				[RenderTargetResource](FRHICommandListImmediate& RHICmdList)
				{
					RenderTargetResource->FlushDeferredResourceUpdate(RHICmdList);
				});

			Canvas->K2_DrawMaterial(Material, FVector2D(0, 0), FVector2D(TextureRenderTarget->SizeX, TextureRenderTarget->SizeY), FVector2D(0, 0));

			RenderCanvas.Flush_GameThread();
			Canvas->Canvas = nullptr;

			//UpdateResourceImmediate must be called here to ensure mips are generated.
			TextureRenderTarget->UpdateResourceImmediate(false);
		}
	}
}


UReprojectionCalibrationStereolabsTarget::UReprojectionCalibrationStereolabsTarget()
{
	static const FString InvTonemappingMatRef = "/Script/Engine.Material'/StereolabsCompositing/StereolabsCompositing/Materials/M_StereolabsInverseTonemapping.M_StereolabsInverseTonemapping'";
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> InvTonemappingMatFinder(*InvTonemappingMatRef);

	if (InvTonemappingMatFinder.Succeeded())
	{
		InverseTonemappingMaterial = Cast<UMaterialInterface>(InvTonemappingMatFinder.Object);
	}
	else
	{
		UE_LOG(LogStereolabsCompositing, Error, TEXT("Failed to find inverse tonemapping material!"));
	}
}

#if WITH_EDITOR

void UReprojectionCalibrationStereolabsTarget::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if we need to request a new source from the engine subsystem
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	bool TargetChanged = PropertyName == GET_MEMBER_NAME_CHECKED(UReprojectionCalibrationStereolabsTarget, InputType);
	TargetChanged	  |= PropertyName == GET_MEMBER_NAME_CHECKED(UReprojectionCalibrationStereolabsTarget, ViewSource);
	TargetChanged	  |= PropertyName == GET_MEMBER_NAME_CHECKED(UReprojectionCalibrationStereolabsTarget, MeasureSource);
	if (TargetChanged)
	{
		FetchNewWrapper();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

TObjectPtr<UTexture> UReprojectionCalibrationStereolabsTarget::GetTexture()
{
	if (!ImageWrapper.IsValid())
	{
		FetchNewWrapper();
	}

	if (!ImageWrapper.IsValid())
	{
		return nullptr;
	}

	return bHasValidTonemappedTexture ? static_cast<TObjectPtr<UTexture>>(InverseTonemappedTexture) : static_cast<TObjectPtr<UTexture>>(ImageWrapper->GetTexture());
}

// TODO: The logic in this function is horrific.
// TODO: It will be much cleaner once tonemapping is handled internally by SlCompEngineSubsystem
// TODO: Potentially ticking textures will be completely redundant at that point
bool UReprojectionCalibrationStereolabsTarget::TickTexture()
{
	if (!ImageWrapper)
		return false;

	auto Texture = ImageWrapper->GetTexture();
	if (Texture && bInverseTonemapping)
	{
		// Render texture from image wrapper to another texture using specified material
		if (!InverseTonemappedTexture)
		{
			CreateInverseTonemappedTexture(Texture);
		}
		if (!CanvasForDrawMaterialToRenderTarget)
		{
			CanvasForDrawMaterialToRenderTarget = NewObject<UCanvas>(GetTransientPackage(), NAME_None);
		}
		if (!InverseTonemappingMID)
		{
			// Create MID
			InverseTonemappingMID = UMaterialInstanceDynamic::Create(InverseTonemappingMaterial, this);
		}

		InverseTonemappingMID->SetTextureParameterValue("Color", Texture);
		::DrawMaterialToRenderTarget(CanvasForDrawMaterialToRenderTarget, InverseTonemappedTexture, InverseTonemappingMID);
		bHasValidTonemappedTexture = true;
		return true;
	}
	else if (bHasValidTonemappedTexture)
	{
		// Tonemapping has been turned off - need to switch textures
		bHasValidTonemappedTexture = false;
		return true;
	}

	return bInverseTonemapping != bHasValidTonemappedTexture;
}

void UReprojectionCalibrationStereolabsTarget::FetchNewWrapper()
{
	bHasValidTonemappedTexture = false;
	InverseTonemappedTexture = nullptr;

	if (auto Subsystem = GEngine->GetEngineSubsystem<USlCompEngineSubsystem>())
	{
		switch (InputType)
		{
			case ESlCompInputType::SlComp_View:		ImageWrapper = Subsystem->GetOrCreateImageWrapper(ViewSource);		break;
			case ESlCompInputType::SlComp_Measure:	ImageWrapper = Subsystem->GetOrCreateImageWrapper(MeasureSource);	break;
			default:
			{
				checkNoEntry();
			}
		}
	}
	else
	{
		ImageWrapper = nullptr;
	}
}

void UReprojectionCalibrationStereolabsTarget::CreateInverseTonemappedTexture(TObjectPtr<UTexture2D> InTexture)
{
	ETextureRenderTargetFormat Format;
	if (!GetTargetFormatFromPixelFormat(InTexture->GetPixelFormat(), Format))
	{
		// Fallback format
		Format = RTF_RGBA8;
	}

	InverseTonemappedTexture = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
	check(InverseTonemappedTexture);
	InverseTonemappedTexture->RenderTargetFormat = Format;
	InverseTonemappedTexture->ClearColor = FLinearColor::Black;
	InverseTonemappedTexture->bAutoGenerateMips = false;
	InverseTonemappedTexture->bCanCreateUAV = false;
	InverseTonemappedTexture->InitAutoFormat(InTexture->GetSizeX(), InTexture->GetSizeY());
	InverseTonemappedTexture->UpdateResourceImmediate(true);
}
