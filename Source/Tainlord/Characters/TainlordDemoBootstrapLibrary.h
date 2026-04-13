#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Characters/TainlordOnboardingLibrary.h"
#include "Progression/TainlordDemoClassLibrary.h"
#include "TainlordDemoBootstrapLibrary.generated.h"

USTRUCT(BlueprintType)
struct FTainlordStarterProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	ETainlordArchetype Archetype = ETainlordArchetype::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FName RaceId = "human";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FName CityId = "aztec_capital";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FString CharacterName = TEXT("TainlordHero");
};

USTRUCT(BlueprintType)
struct FTainlordBootstrapResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FTainlordStarterProfile StarterProfile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FTainlordClassDefinition ClassDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FTainlordOnboardingPackage OnboardingPackage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	TArray<FTainlordTutorialStepDefinition> TutorialSteps;
};

UCLASS()
class TAINLORD_API UTainlordDemoBootstrapLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tainlord|Bootstrap")
	static FTainlordStarterProfile GetRecommendedStarterProfile();

	UFUNCTION(BlueprintPure, Category = "Tainlord|Bootstrap")
	static FTainlordStarterProfile SanitizeStarterProfile(const FTainlordStarterProfile& RequestedProfile);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Bootstrap")
	static FTainlordBootstrapResult BuildBootstrapResult(const FTainlordStarterProfile& RequestedProfile);
};
