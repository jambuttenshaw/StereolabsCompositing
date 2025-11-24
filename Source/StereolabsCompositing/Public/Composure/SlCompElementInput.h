#pragma once

#include "CoreMinimal.h"

#include "Composure/CompUtilsElementInput.h"
#include "Composure/Classes/CompositingElements/CompositingElementPasses.h"

#include "Core/StereolabsBaseTypes.h"

#include "SlCompElementInput.generated.h"


class FSlCompImageWrapper;

UENUM(BlueprintType)
enum class ESlCompInputType : uint8
{
	SlComp_View			UMETA(DisplayName = "View"),
	SlComp_Measure		UMETA(DisplayName = "Measure"),
};


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API USlCompInput : public UCompositionUtilsAuxiliaryCameraInput
{
	GENERATED_BODY()
public:
	USlCompInput();

private:
	// Private as modifying these members will change which ImageWrapper this input element should point to

	UPROPERTY(EditAnywhere, Category="Compositing Pass", meta=(EditCondition="bEnabled"))
	ESlCompInputType InputType = ESlCompInputType::SlComp_View;

	UPROPERTY(EditAnywhere, Category="Compositing Pass", meta=(EditCondition="bEnabled&&InputType==ESlCompInputType::SlComp_View"))
	ESlView ViewSource = ESlView::V_Left;

	UPROPERTY(EditAnywhere, Category="Compositing Pass", meta=(EditCondition="bEnabled&&InputType==ESlCompInputType::SlComp_Measure"))
	ESlMeasure MeasureSource = ESlMeasure::M_Depth;

public:
	//~ Begin UCompositionUtilsAuxiliaryCameraInput interface
	virtual bool GetCameraData(FAuxiliaryCameraData& OutData) override;
	//~ End UCompositionUtilsAuxiliaryCameraInput interface

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
