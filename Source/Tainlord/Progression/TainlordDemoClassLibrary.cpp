#include "Progression/TainlordDemoClassLibrary.h"

namespace
{
	FTainlordClassDefinition MakeDemoClass(
		const ETainlordArchetype Archetype,
		const FText& DisplayName,
		const ETainlordCombatRole PrimaryRole,
		const ETainlordStatOrientation DefaultOrientation,
		const TArray<ETainlordStatOrientation>& AllowedOrientations,
		const TArray<ETainlordElement>& RecommendedElements,
		const FText& Summary)
	{
		FTainlordClassDefinition Definition;
		Definition.Archetype = Archetype;
		Definition.DisplayName = DisplayName;
		Definition.PrimaryRole = PrimaryRole;
		Definition.DefaultOrientation = DefaultOrientation;
		Definition.AllowedOrientations = AllowedOrientations;
		Definition.RecommendedElements = RecommendedElements;
		Definition.bIncludedInFundingDemo = true;
		Definition.ClassSummary = Summary;
		return Definition;
	}
}

TArray<FTainlordClassDefinition> UTainlordDemoClassLibrary::GetFundingDemoClassDefinitions()
{
	return {
		MakeDemoClass(
			ETainlordArchetype::Warrior,
			NSLOCTEXT("Tainlord", "WarriorClassName", "Warrior"),
			ETainlordCombatRole::Frontline,
			ETainlordStatOrientation::Strength,
			{ ETainlordStatOrientation::Strength },
			{ ETainlordElement::Earth, ETainlordElement::Fire },
			NSLOCTEXT("Tainlord", "WarriorClassSummary", "Durable melee frontliner with controlled engage windows and readable physical pressure.")),
		MakeDemoClass(
			ETainlordArchetype::Mage,
			NSLOCTEXT("Tainlord", "MageClassName", "Mage"),
			ETainlordCombatRole::RangedDamage,
			ETainlordStatOrientation::Intelligence,
			{ ETainlordStatOrientation::Intelligence },
			{ ETainlordElement::Fire, ETainlordElement::Water },
			NSLOCTEXT("Tainlord", "MageClassSummary", "High-readability ranged damage dealer with elemental burst and spacing pressure.")),
		MakeDemoClass(
			ETainlordArchetype::Cleric,
			NSLOCTEXT("Tainlord", "ClericClassName", "Cleric"),
			ETainlordCombatRole::SustainSupport,
			ETainlordStatOrientation::Intelligence,
			{ ETainlordStatOrientation::Intelligence, ETainlordStatOrientation::Hybrid },
			{ ETainlordElement::Water, ETainlordElement::Earth },
			NSLOCTEXT("Tainlord", "ClericClassSummary", "Slow, high-impact sustain support with cleanse, stat support, and anti-collapse recovery."))
	};
}

FTainlordClassDefinition UTainlordDemoClassLibrary::GetClassDefinition(const ETainlordArchetype Archetype)
{
	for (const FTainlordClassDefinition& Definition : GetFundingDemoClassDefinitions())
	{
		if (Definition.Archetype == Archetype)
		{
			return Definition;
		}
	}

	FTainlordClassDefinition FallbackDefinition;
	FallbackDefinition.Archetype = Archetype;
	FallbackDefinition.DisplayName = UEnum::GetDisplayValueAsText(Archetype);
	return FallbackDefinition;
}


