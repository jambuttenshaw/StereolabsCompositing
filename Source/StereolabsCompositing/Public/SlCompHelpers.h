#pragma once

#include "CoreMinimal.h"
#include "CompUtilsCameraData.h"
#include "Core/StereolabsBaseTypes.h"


// TODO: Could be useful as a blueprint function library
class FSlCompHelpers
{
	// Static class with helper functions - not to be instantiated 
	FSlCompHelpers() = delete;

public:
	static bool IsCameraOpen();

	static const FSlCameraParameters& GetCameraParameters(ESlView View);
	static const FSlCameraParameters& GetCameraParameters(ESlMeasure Measure);

	static bool GetCameraIntrinsicData(ESlView View, FCompUtilsCameraIntrinsicData& OutIntrinsicData);
	static bool GetCameraIntrinsicData(ESlMeasure Measure, FCompUtilsCameraIntrinsicData& OutIntrinsicData);

private:
	static const FSlCameraParameters& GetLeftCameraParameters();
	static const FSlCameraParameters& GetRightCameraParameters();
	static const FSlCameraParameters& GetLeftUnrectifiedCameraParameters();
	static const FSlCameraParameters& GetRightUnrectifiedCameraParameters();

public:
	static FMatrix GetViewToNDCMatrix(const FSlCameraParameters& CameraParams);
	static FMatrix GetNDCToViewMatrix(const FSlCameraParameters& CameraParams);
};
