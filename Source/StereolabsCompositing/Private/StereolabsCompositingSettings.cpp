// Fill out your copyright notice in the Description page of Project Settings.
#include "StereolabsCompositingSettings.h"

#include "Core/StereolabsCameraProxy.h"
#include "Core/StereolabsCoreGlobals.h"

UStereolabsCompositingSettings::UStereolabsCompositingSettings(const FObjectInitializer& obj)
{
}

#if WITH_EDITOR
void UStereolabsCompositingSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (GSlCameraProxy)
	{
		GSlCameraProxy->OpenCamera(InitParams);
	}
}
#endif