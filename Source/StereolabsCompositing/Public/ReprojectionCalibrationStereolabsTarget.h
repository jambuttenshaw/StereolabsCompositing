#pragma once

#include "CoreMinimal.h"
#include "ReprojectionCalibration.h"

#include "Core/StereolabsBaseTypes.h"
#include "SlCompTypes.h"

#include "SlCompEngineSubsystem.h"

#include "ReprojectionCalibrationStereolabsTarget.generated.h"


/**
 * Stereolabs Compositing: Represents an image from Stereolabs ZED camera as a target for reprojection
 */
UCLASS()
class STEREOLABSCOMPOSITING_API UReprojectionCalibrationStereolabsTarget : public UReprojectionCalibrationTargetBase
{
	GENERATED_BODY()

public:
	UReprojectionCalibrationStereolabsTarget();

private:
	// Private as modifying these members will change which ImageWrapper this input element should point to
	// TODO: Add UFUNCTION's to allow blueprint to interface

	UPROPERTY(EditAnywhere)
	ESlCompInputType InputType = ESlCompInputType::SlComp_View;

	UPROPERTY(EditAnywhere, meta = (EditCondition="InputType==ESlCompInputType::SlComp_View"))
	ESlView ViewSource = ESlView::V_Left;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "InputType==ESlCompInputType::SlComp_Measure"))
	ESlMeasure MeasureSource = ESlMeasure::M_Depth;

	UPROPERTY(EditAnywhere)
	bool bInverseTonemapping = false;

protected:
	//~ Begin UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject interface

public:
	//~ Begin UReprojectionCalibrationTargetBase interface
	virtual TObjectPtr<UTexture> GetTexture() override;
	virtual bool TickTexture() override;
	//~ End UReprojectionCalibrationTargetBase interface

private:
	// Wrapper is left as nullptr if failed
	void FetchNewWrapper();

	// Populates InverseTonemappedTexture with a texture based off of InTexture (same format, dimensions)
	void CreateInverseTonemappedTexture(TObjectPtr<UTexture2D> InTexture);

private:
	TSharedPtr<FSlCompImageWrapper> ImageWrapper;

	bool bHasValidTonemappedTexture = false;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> InverseTonemappedTexture;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> InverseTonemappingMaterial;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> InverseTonemappingMID;

	// TODO: Move this to engine subsystem to avoid having canvas per instance of this object
	UPROPERTY(Transient)
	TObjectPtr<UCanvas> CanvasForDrawMaterialToRenderTarget;
};
