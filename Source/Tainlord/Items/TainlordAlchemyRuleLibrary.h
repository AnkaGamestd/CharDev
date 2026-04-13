#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Domain/TainlordGameplayTypes.h"
#include "TainlordAlchemyRuleLibrary.generated.h"

UCLASS()
class TAINLORD_API UTainlordAlchemyRuleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tainlord|Alchemy")
	static FTainlordGemDefinition MakeStarterGem(ETainlordArchetype Archetype);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Alchemy")
	static FTainlordAlchemyCombineResult CombineGems(ETainlordGemSize InputSize, int32 InputCount);
};
