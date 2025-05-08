#pragma once

#include "CoreMinimal.h"
#include "Composure/Classes/CompositingElements/CompositingElementPasses.h"

#include "StereolabsCompositingElementInput.generated.h"


UENUM(BlueprintType)
enum class EStereolabsCompositingInputChannel : uint8
{
	Color,
	Depth,
	Normal
};


UCLASS(BlueprintType, Blueprintable)
class STEREOLABSCOMPOSITING_API UStereolabsCompositingInput : public UCompositingElementInput
{
	GENERATED_BODY()
public:
	UStereolabsCompositingInput();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Compositing Pass", meta=(EditCondition="bEnabled"))
	EStereolabsCompositingInputChannel InputSource = EStereolabsCompositingInputChannel::Color;

protected:
	//~ Begin UCompositingElementInput interface	
	virtual UTexture* GenerateInput_Implementation() override;
	//~ End UCompositingElementInput interface

private:

};
