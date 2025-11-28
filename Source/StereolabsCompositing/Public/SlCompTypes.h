#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ESlCompInputType : uint8
{
	SlComp_View			UMETA(DisplayName = "View"),
	SlComp_Measure		UMETA(DisplayName = "Measure"),
};
