#include "Items/TainlordAlchemyRuleLibrary.h"

FTainlordGemDefinition UTainlordAlchemyRuleLibrary::MakeStarterGem(const ETainlordArchetype Archetype)
{
	FTainlordGemDefinition Gem;
	Gem.Size = ETainlordGemSize::Mini;
	Gem.ModifierPercent = 5.0f;

	switch (Archetype)
	{
	case ETainlordArchetype::Warrior:
		Gem.GemId = "mini_ruby";
		Gem.DisplayName = NSLOCTEXT("Tainlord", "MiniRuby", "Mini Ruby");
		Gem.ModifierType = ETainlordGemModifierType::PhysicalPower;
		break;
	case ETainlordArchetype::Mage:
		Gem.GemId = "mini_sapphire";
		Gem.DisplayName = NSLOCTEXT("Tainlord", "MiniSapphire", "Mini Sapphire");
		Gem.ModifierType = ETainlordGemModifierType::SpellPower;
		break;
	case ETainlordArchetype::Cleric:
		Gem.GemId = "mini_emerald";
		Gem.DisplayName = NSLOCTEXT("Tainlord", "MiniEmerald", "Mini Emerald");
		Gem.ModifierType = ETainlordGemModifierType::Willpower;
		break;
	default:
		Gem.GemId = "mini_quartz";
		Gem.DisplayName = NSLOCTEXT("Tainlord", "MiniQuartz", "Mini Quartz");
		Gem.ModifierType = ETainlordGemModifierType::Defense;
		break;
	}

	return Gem;
}

FTainlordAlchemyCombineResult UTainlordAlchemyRuleLibrary::CombineGems(const ETainlordGemSize InputSize, const int32 InputCount)
{
	FTainlordAlchemyCombineResult Result;
	Result.ConsumedCount = FMath::Max(0, InputCount);
	Result.ResultSize = InputSize;

	if (InputCount < 3)
	{
		return Result;
	}

	Result.bSuccess = true;

	switch (InputSize)
	{
	case ETainlordGemSize::Mini:
		Result.ResultSize = ETainlordGemSize::Small;
		break;
	case ETainlordGemSize::Small:
		Result.ResultSize = ETainlordGemSize::Medium;
		break;
	case ETainlordGemSize::Medium:
		Result.ResultSize = ETainlordGemSize::Large;
		break;
	case ETainlordGemSize::Large:
	default:
		Result.ResultSize = ETainlordGemSize::Large;
		break;
	}

	return Result;
}
