#include "Characters/TainlordOnboardingLibrary.h"

#include "Combat/TainlordCombatRuleLibrary.h"
#include "Items/TainlordAlchemyRuleLibrary.h"
#include "Items/TainlordItemRuleLibrary.h"
#include "Progression/TainlordProgressionRuleLibrary.h"

namespace
{
	FTainlordCityDefinition MakeCity(const FName CityId, const FText& DisplayName, const FText& Summary, const bool bRecommendedForNewPlayers)
	{
		FTainlordCityDefinition City;
		City.CityId = CityId;
		City.DisplayName = DisplayName;
		City.ShortDescription = Summary;
		City.bIncludedInFundingDemo = true;
		City.bRecommendedForNewPlayers = bRecommendedForNewPlayers;
		return City;
	}

	FTainlordTutorialStepDefinition MakeStep(const FName StepId, const FText& Title, const FText& Instruction)
	{
		FTainlordTutorialStepDefinition Step;
		Step.StepId = StepId;
		Step.Title = Title;
		Step.Instruction = Instruction;
		Step.bRequired = true;
		return Step;
	}
}

TArray<FTainlordCityDefinition> UTainlordOnboardingLibrary::GetFundingDemoCities()
{
	return {
		MakeCity(
			"aztec_capital",
			NSLOCTEXT("Tainlord", "AztecCapitalName", "Xolatl"),
			NSLOCTEXT("Tainlord", "AztecCapitalSummary", "Dense jungle capital with strong market life and direct access to the demo conflict route."),
			true),
		MakeCity(
			"exile_outpost",
			NSLOCTEXT("Tainlord", "ExileOutpostName", "Exile Outpost"),
			NSLOCTEXT("Tainlord", "ExileOutpostSummary", "Low-support frontier shelter for future exile play. Visible in demo, but not the recommended first route."),
			false)
	};
}

TArray<FTainlordTutorialStepDefinition> UTainlordOnboardingLibrary::GetFundingDemoTutorialSteps()
{
	return {
		MakeStep(
			"enter_world",
			NSLOCTEXT("Tainlord", "TutorialEnterWorldTitle", "Enter The World"),
			NSLOCTEXT("Tainlord", "TutorialEnterWorldInstruction", "Leave the city gate and step into the first combat route.")),
		MakeStep(
			"first_combat",
			NSLOCTEXT("Tainlord", "TutorialFirstCombatTitle", "Use Your Starter Skills"),
			NSLOCTEXT("Tainlord", "TutorialFirstCombatInstruction", "Use your basic attack and first signature skill to clear the first enemy pack.")),
		MakeStep(
			"equip_weapon",
			NSLOCTEXT("Tainlord", "TutorialEquipWeaponTitle", "Equip Your Weapon"),
			NSLOCTEXT("Tainlord", "TutorialEquipWeaponInstruction", "Open the inventory and equip your starter weapon to feel the first power jump.")),
		MakeStep(
			"socket_gem",
			NSLOCTEXT("Tainlord", "TutorialSocketGemTitle", "Use Your First Gem"),
			NSLOCTEXT("Tainlord", "TutorialSocketGemInstruction", "Inspect your first gem reward and prepare for the first alchemy or socketing touchpoint."))
	};
}

FTainlordOnboardingPackage UTainlordOnboardingLibrary::BuildFundingDemoOnboardingPackage(const ETainlordArchetype Archetype, const FName RaceId, const FName CityId)
{
	FTainlordOnboardingPackage Package;
	Package.StartingCityId = CityId;
	Package.IntroQuestId = "demo_intro_route";

	Package.CharacterState.Allegiance = (CityId == "exile_outpost") ? ETainlordAllegiance::Exile : ETainlordAllegiance::City;
	Package.CharacterState.Progression.Archetype = Archetype;
	Package.CharacterState.Progression.StatOrientation = (Archetype == ETainlordArchetype::Warrior)
		? ETainlordStatOrientation::Strength
		: ETainlordStatOrientation::Intelligence;
	Package.CharacterState.Progression.Level = 1;
	Package.CharacterState.Progression.Experience = 0;
	Package.CharacterState.Progression.ExperienceToNextLevel = UTainlordProgressionRuleLibrary::GetExperienceRequirementForLevel(1);
	Package.CharacterState.Customization.RaceId = RaceId;
	Package.CharacterState.Wallet = UTainlordProgressionRuleLibrary::BuildStarterWallet();
	Package.CharacterState.DerivedStats = UTainlordCombatRuleLibrary::BuildDerivedStats(Package.CharacterState);
	Package.CharacterState.CombatResources = UTainlordCombatRuleLibrary::BuildCombatResources(Package.CharacterState);

	Package.StarterWeapon.EntryId = FName(*FString::Printf(TEXT("starter_%s_weapon"), *UEnum::GetValueAsString(Archetype)));
	Package.StarterWeapon.Item = UTainlordItemRuleLibrary::MakeStarterWeaponDefinition(Archetype);
	Package.StarterWeapon.Quantity = 1;

	Package.StarterGem = UTainlordAlchemyRuleLibrary::MakeStarterGem(Archetype);
	Package.CharacterState.Equipment = UTainlordItemRuleLibrary::EquipItem(Package.CharacterState.Equipment, Package.StarterWeapon);

	return Package;
}
