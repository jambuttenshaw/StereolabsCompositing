#pragma once

#include "CoreMinimal.h"

#include "Composure/Classes/CompositingElements/CompositingElementPasses.h"
#include "CompUtilsCameraInterface.h"

#include "Core/StereolabsBaseTypes.h"
#include "SlCompTypes.h"

#include "SlCompElementInput.generated.h"


class FSlCompImageWrapper;


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API USlCompInput : public UCompositingElementInput, public ICompUtilsCameraInterface
{
	GENERATED_BODY()
public:
	USlCompInput();

private:
	// Private as modifying these members will change which ImageWrapper this input element should point to
	// TODO: Add UFUNCTION's to allow blueprint to interface

	UPROPERTY(EditAnywhere, Category="Compositing Pass", meta=(EditCondition="bEnabled"))
	ESlCompInputType InputType = ESlCompInputType::SlComp_View;

	UPROPERTY(EditAnywhere, Category="Compositing Pass", meta=(EditCondition="bEnabled&&InputType==ESlCompInputType::SlComp_View"))
	ESlView ViewSource = ESlView::V_Left;

	UPROPERTY(EditAnywhere, Category="Compositing Pass", meta=(EditCondition="bEnabled&&InputType==ESlCompInputType::SlComp_Measure"))
	ESlMeasure MeasureSource = ESlMeasure::M_Depth;

public:
	//~ Begin ICompUtilsCameraInterface interface
	virtual bool GetCameraIntrinsicData(FCompUtilsCameraIntrinsicData& OutData) override;
	//~ End ICompUtilsCameraInterface interface

protected:
	//~ Begin UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject interface

protected:
	//~ Begin UCompositingElementInput interface	
	virtual UTexture* GenerateInput_Implementation() override;
	//~ End UCompositingElementInput interface

private:

	// Wrapper is left as nullptr if failed
	void FetchNewWrapper();

private:
	TSharedPtr<FSlCompImageWrapper> ImageWrapper;

};
