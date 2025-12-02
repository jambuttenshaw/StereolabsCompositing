#include "SlCompHelpers.h"

#include "StereolabsCompositing.h"
#include "Core/StereolabsCoreGlobals.h"
#include "Core/StereolabsCameraProxy.h"


bool FSlCompHelpers::IsCameraOpen()
{
	return GSlCameraProxy && GSlCameraProxy->IsCameraOpened();
}

const FSlCameraParameters& FSlCompHelpers::GetCameraParameters(ESlView View)
{
	check(IsCameraOpen());

	switch (View)
	{
	case ESlView::V_Left:
	case ESlView::V_Depth:
	case ESlView::V_Confidence:
	case ESlView::V_Normals:
		return GetLeftCameraParameters();

	case ESlView::V_Right:
	case ESlView::V_DepthRight:
	case ESlView::V_NormalsRight:
		return GetRightCameraParameters();

	case ESlView::V_LeftUnrectified:
		return GetLeftUnrectifiedCameraParameters();

	case ESlView::V_RightUnrectified:
		return GetRightUnrectifiedCameraParameters();

	case ESlView::V_SideBySide:
	default:
	{
		FString ViewAsString = EnumToString(View);
		UE_LOG(LogStereolabsCompositing, Error, TEXT("Cannot get parameters for view: %s"), *ViewAsString);
	}
	}

	return GetLeftCameraParameters();
}

const FSlCameraParameters& FSlCompHelpers::GetCameraParameters(ESlMeasure Measure)
{
	check(IsCameraOpen());

	switch (Measure)
	{
	case ESlMeasure::M_Disparity:
	case ESlMeasure::M_Depth:
	case ESlMeasure::M_Confidence:
	case ESlMeasure::M_XYZ:
	case ESlMeasure::M_XYZ_RGBA:
	case ESlMeasure::M_XYZ_BGRA:
	case ESlMeasure::M_XYZ_ARGB:
	case ESlMeasure::M_XYZ_ABGR:
	case ESlMeasure::M_Normals:
		return GetLeftCameraParameters();

	case ESlMeasure::M_DisparityRight:
	case ESlMeasure::M_DepthRight:
	case ESlMeasure::M_XYZ_Right:
	case ESlMeasure::M_XYZ_RGBA_Right:
	case ESlMeasure::M_XYZ_BGRA_Right:
	case ESlMeasure::M_XYZ_ARGB_Right:
	case ESlMeasure::M_XYZ_ABGR_Right:
	case ESlMeasure::M_NormalsRight:
	case ESlMeasure::M_DEPTH_U16_MM:
	case ESlMeasure::M_DEPTH_U16_MM_RIGHT:
		return GetRightCameraParameters();
	}

	// Unhandled case
	checkNoEntry();
	return GetLeftCameraParameters();
}

bool FSlCompHelpers::GetCameraIntrinsicData(ESlView View, FCompUtilsCameraIntrinsicData& OutIntrinsicData)
{
	if (IsCameraOpen())
	{
		OutIntrinsicData.Type = ECompUtilsCameraType::CameraType_Physical;

		const auto& Parameters = GetCameraParameters(View);

		OutIntrinsicData.ViewToNDC = static_cast<FMatrix44f>(GetViewToNDCMatrix(Parameters));
		OutIntrinsicData.NDCToView = OutIntrinsicData.ViewToNDC.Inverse();

		OutIntrinsicData.HorizontalFOV = Parameters.HFOV;
		OutIntrinsicData.VerticalFOV = Parameters.VFOV;

		OutIntrinsicData.FocalLength = FVector2D{ Parameters.HFocal, Parameters.VFocal };
		OutIntrinsicData.ImageCenter = FVector2D{ Parameters.OpticalCenterX, Parameters.OpticalCenterY };
		OutIntrinsicData.DistortionParams.Empty(Parameters.Disto.Num());
		OutIntrinsicData.DistortionParams.Append(Parameters.Disto);

		return true;
	}
	return false;
}

bool FSlCompHelpers::GetCameraIntrinsicData(ESlMeasure Measure, FCompUtilsCameraIntrinsicData& OutIntrinsicData)
{
	if (IsCameraOpen())
	{
		OutIntrinsicData.Type = ECompUtilsCameraType::CameraType_Physical;

		const auto& Parameters = GetCameraParameters(Measure);

		OutIntrinsicData.ViewToNDC = static_cast<FMatrix44f>(GetViewToNDCMatrix(Parameters));
		OutIntrinsicData.NDCToView = OutIntrinsicData.ViewToNDC.Inverse();

		OutIntrinsicData.HorizontalFOV = Parameters.HFOV;
		OutIntrinsicData.VerticalFOV = Parameters.VFOV;

		OutIntrinsicData.FocalLength = FVector2D{ Parameters.HFocal, Parameters.VFocal };
		OutIntrinsicData.ImageCenter = FVector2D{ Parameters.OpticalCenterX, Parameters.OpticalCenterY };
		OutIntrinsicData.DistortionParams.Empty(Parameters.Disto.Num());
		OutIntrinsicData.DistortionParams.Append(Parameters.Disto);

		return true;
	}
	return false;
}

const FSlCameraParameters& FSlCompHelpers::GetLeftCameraParameters()
{
	check(IsCameraOpen());
	return GSlCameraProxy->CameraInformation.CalibrationParameters.LeftCameraParameters;
}

const FSlCameraParameters& FSlCompHelpers::GetRightCameraParameters()
{
	check(IsCameraOpen());
	return GSlCameraProxy->CameraInformation.CalibrationParameters.RightCameraParameters;
}

const FSlCameraParameters& FSlCompHelpers::GetLeftUnrectifiedCameraParameters()
{
	check(IsCameraOpen());
	return GSlCameraProxy->CameraInformation.CalibrationParametersRaw.LeftCameraParameters;
}

const FSlCameraParameters& FSlCompHelpers::GetRightUnrectifiedCameraParameters()
{
	check(IsCameraOpen());
	return GSlCameraProxy->CameraInformation.CalibrationParametersRaw.RightCameraParameters;
}

FMatrix FSlCompHelpers::GetViewToNDCMatrix(const FSlCameraParameters& CameraParams)
{
	check(IsCameraOpen());

	// Calculate a projection matrix based off of the camera properties
	// TODO: Only correct for undistorted views

	float HorizontalFieldOfView = FMath::DegreesToRadians(CameraParams.HFOV);
	float VerticalFieldOfView = FMath::DegreesToRadians(CameraParams.VFOV);

	// Introduction of a near plane (0.1cm) is required to make the matrix invertible
	// The choice of near plane doesn't matter - as we don't ever care about the depth value after projection,
	// and we use the data sampled from the depth texture to un-project anyway.
	const float HalfFovX = 0.5f * HorizontalFieldOfView;
	const float HalfFovY = 0.5f * VerticalFieldOfView;
	return FMatrix(
		FPlane(1.0f / FMath::Tan(HalfFovX), 0.0f, 0.0f, 0.0f),
		FPlane(0.0f, 1.0f / FMath::Tan(HalfFovY), 0.0f, 0.0f),
		FPlane(0.0f, 0.0f, 0.0f, 1.0f),
		FPlane(0.0f, 0.0f, 0.1f, 0.0f)
	);
}

FMatrix FSlCompHelpers::GetNDCToViewMatrix(const FSlCameraParameters& CameraParams)
{
	check(IsCameraOpen());
	return GetViewToNDCMatrix(CameraParams).Inverse();
}
