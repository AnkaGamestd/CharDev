#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Domain/TainlordGameplayTypes.h"
#include "TainlordDemoClassLibrary.generated.h"

UCLASS()
class TAINLORD_API UTainlordDemoClassLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tainlord|Classes")
	static TArray<FTainlordClassDefinition> GetFundingDemoClassDefinitions();

	UFUNCTION(BlueprintPure, Category = "Tainlord|Classes")
	static FTainlordClassDefinition GetClassDefinition(ETainlordArchetype Archetype);
};
