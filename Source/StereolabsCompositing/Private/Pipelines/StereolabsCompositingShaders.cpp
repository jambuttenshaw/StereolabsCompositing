#include "StereolabsCompositingShaders.h"


IMPLEMENT_GLOBAL_SHADER(FPreProcessDepthPS, "/Plugin/StereolabsCompositing/DepthProcessing.usf", "PreProcessDepthPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FRestrictPS, "/Plugin/StereolabsCompositing/DepthProcessing.usf", "RestrictPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FInterpolatePS, "/Plugin/StereolabsCompositing/DepthProcessing.usf", "InterpolatePS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FJacobiStepPS, "/Plugin/StereolabsCompositing/DepthProcessing.usf", "JacobiStepPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FDepthClippingPS, "/Plugin/StereolabsCompositing/DepthProcessing.usf", "DepthClipPS", SF_Pixel);


IMPLEMENT_GLOBAL_SHADER(FVolumetricCompositionPS, "/Plugin/StereolabsCompositing/VolumetricComposition.usf", "VolumetricCompositionPS", SF_Pixel);


IMPLEMENT_GLOBAL_SHADER(FRelightingPS, "/Plugin/StereolabsCompositing/Relighting.usf", "RelightingPS", SF_Pixel);
