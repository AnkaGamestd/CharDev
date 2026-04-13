#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Domain/TainlordGameplayTypes.h"
#include "Domain/TainlordCityTypes.h"
#include "TainlordOnboardingLibrary.generated.h"

USTRUCT(BlueprintType)
struct FTainlordTutorialStepDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	FName StepId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	FText Instruction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	bool bRequired = true;
};

USTRUCT(BlueprintType)
struct FTainlordOnboardingPackage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Onboarding")
	FTainlordCharacterState CharacterState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Onboarding")
	FTainlordInventoryEntry StarterWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Onboarding")
	FTainlordGemDefinition StarterGem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Onboarding")
	FName StartingCityId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Onboarding")
	FName IntroQuestId = NAME_None;
};

UCLASS()
class TAINLORD_API UTainlordOnboardingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tainlord|Onboarding")
	static TArray<FTainlordCityDefinition> GetFundingDemoCities();

	UFUNCTION(BlueprintPure, Category = "Tainlord|Onboarding")
	static TArray<FTainlordTutorialStepDefinition> GetFundingDemoTutorialSteps();

	UFUNCTION(BlueprintPure, Category = "Tainlord|Onboarding")
	static FTainlordOnboardingPackage BuildFundingDemoOnboardingPackage(ETainlordArchetype Archetype, FName RaceId, FName CityId);
};
