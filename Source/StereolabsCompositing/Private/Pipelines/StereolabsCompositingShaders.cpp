#include "StereolabsCompositingShaders.h"


IMPLEMENT_GLOBAL_SHADER(FPreProcessDepthPS, "/Plugin/StereolabsCompositing/DepthRelaxation.usf", "PreProcessDepthPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FRestrictPS, "/Plugin/StereolabsCompositing/DepthRelaxation.usf", "RestrictPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FInterpolatePS, "/Plugin/StereolabsCompositing/DepthRelaxation.usf", "InterpolatePS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FJacobiStepPS, "/Plugin/StereolabsCompositing/DepthRelaxation.usf", "JacobiStepPS", SF_Pixel);
