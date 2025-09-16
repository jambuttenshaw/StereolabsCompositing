#pragma once

#include "CoreMinimal.h"
#include "Composure/CompUtilsElementInput.h"
#include "Composure/Classes/CompositingElements/CompositingElementPasses.h"

#include "SlCompElementInput.generated.h"


UENUM(BlueprintType)
enum class ESlCompInputChannel : uint8
{
	Color,
	Depth,
	Normal
};


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API USlCompInput : public UCompositionUtilsAuxiliaryCameraInput
{
	GENERATED_BODY()
public:
	USlCompInput();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Compositing Pass", meta=(EditCondition="bEnabled"))
	ESlCompInputChannel InputSource = ESlCompInputChannel::Color;

	//~ Begin UCompositionUtilsAuxiliaryCameraInput interface
	virtual bool GetCameraData(FAuxiliaryCameraDataProxy& OutData) override;
	//~ End UCompositionUtilsAuxiliaryCameraInput interface

protected:
	//~ Begin UCompositingElementInput interface	
	virtual UTexture* GenerateInput_Implementation() override;
	//~ End UCompositingElementInput interface

private:

};
