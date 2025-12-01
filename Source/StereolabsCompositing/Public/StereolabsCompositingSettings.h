// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/StereolabsBaseTypes.h"

#include "StereolabsCompositingSettings.generated.h"

/**
 * 
 */
UCLASS(config = StereolabsSettings)
class STEREOLABSCOMPOSITING_API UStereolabsCompositingSettings : public UObject
{
	GENERATED_BODY()

public:
	UStereolabsCompositingSettings(const FObjectInitializer& obj);

#if WITH_EDITOR
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(Config, EditAnywhere, Category = "Stereolabs Settigns")
	FSlInitParameters InitParams;
};
