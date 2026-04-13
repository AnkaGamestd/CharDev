#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Domain/TainlordGameplayTypes.h"
#include "TainlordProgressionRuleLibrary.generated.h"

UCLASS()
class TAINLORD_API UTainlordProgressionRuleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tainlord|Progression")
	static int32 GetExperienceRequirementForLevel(int32 Level);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Progression")
	static FTainlordProgressionState GrantExperience(const FTainlordProgressionState& ProgressionState, int32 ExperienceGain);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Progression")
	static FTainlordCurrencyWallet BuildStarterWallet();
};
