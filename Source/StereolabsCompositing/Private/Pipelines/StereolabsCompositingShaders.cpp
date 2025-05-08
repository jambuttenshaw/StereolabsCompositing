#include "StereolabsCompositingShaders.h"


IMPLEMENT_GLOBAL_SHADER(FPreProcessDepthPS, "/Plugin/Stereolabs/DepthRelaxation.usf", "PreProcessDepthPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FRestrictPS, "/Plugin/Stereolabs/DepthRelaxation.usf", "RestrictPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FInterpolatePS, "/Plugin/Stereolabs/DepthRelaxation.usf", "InterpolatePS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FJacobiStepPS, "/Plugin/Stereolabs/DepthRelaxation.usf", "JacobiStepPS", SF_Pixel);
