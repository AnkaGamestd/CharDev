#include "Combat/TainlordCombatRuleLibrary.h"

// ---------------------------------------------------------------------------
// Mastery Bridge: Code-based mapping for demo masteries
// ---------------------------------------------------------------------------

namespace
{
	// Internal mapping struct for Tainlord mastery -> KingdawnCombat identity
	struct FMasteryMappingEntry
	{
		FName MasteryId;
		EKDMasteryClass MasteryClass;
		EKDMasteryBranch MasteryBranch;
		ETainlordArchetype Archetype;
		ETainlordStatOrientation StatOrientation;
	};

	// Demo mastery mapping table - single source of truth for v1 bridge
	// This is a pragmatic code-based mapping until full mastery catalog assets are ready
	static const TArray<FMasteryMappingEntry>& GetMasteryMappingTable()
	{
		static TArray<FMasteryMappingEntry> MappingTable;
		
		if (MappingTable.Num() == 0)
		{
			// Warrior masteries
			MappingTable.Add({ FName("Berserker"), EKDMasteryClass::Warrior, EKDMasteryBranch::BladeShield, ETainlordArchetype::Warrior, ETainlordStatOrientation::Strength });
			MappingTable.Add({ FName("Guardian"),  EKDMasteryClass::Warrior, EKDMasteryBranch::BladeShield, ETainlordArchetype::Warrior, ETainlordStatOrientation::Strength });
			
			// Mage masteries
			MappingTable.Add({ FName("Arcanist"), EKDMasteryClass::Mage,      EKDMasteryBranch::Wizard,      ETainlordArchetype::Mage,    ETainlordStatOrientation::Intelligence });
			
			// Support masteries
			MappingTable.Add({ FName("Cleric"),    EKDMasteryClass::Support,   EKDMasteryBranch::Cleric,      ETainlordArchetype::Cleric,  ETainlordStatOrientation::Intelligence });
			
			// Archer masteries
			MappingTable.Add({ FName("Ranger"),    EKDMasteryClass::Archer,    EKDMasteryBranch::Archer,      ETainlordArchetype::Archer,  ETainlordStatOrientation::Strength });
			
			// Rogue masteries
			MappingTable.Add({ FName("Shadowblade"), EKDMasteryClass::Rogue,   EKDMasteryBranch::Dagger,      ETainlordArchetype::Dagger,  ETainlordStatOrientation::Hybrid });
		}
		
		return MappingTable;
	}

	const FMasteryMappingEntry* FindMappingEntry(FName MasteryId)
	{
		const TArray<FMasteryMappingEntry>& Table = GetMasteryMappingTable();
		return Table.FindByPredicate([MasteryId](const FMasteryMappingEntry& Entry)
		{
			return Entry.MasteryId == MasteryId;
		});
	}
}

// ---------------------------------------------------------------------------
// Mastery Bridge Functions
// ---------------------------------------------------------------------------

FKDMasteryIdentity UTainlordCombatRuleLibrary::MapMasteryIdToCombatIdentity(FName MasteryId)
{
	if (const FMasteryMappingEntry* Entry = FindMappingEntry(MasteryId))
	{
		return FKDMasteryIdentity(Entry->MasteryClass, Entry->MasteryBranch);
	}
	
	// Return invalid identity if mastery not found
	UE_LOG(LogTemp, Warning, TEXT("TainlordCombatRule: Unknown mastery ID '%s' - returning invalid FKDMasteryIdentity"), *MasteryId.ToString());
	return FKDMasteryIdentity();
}

bool UTainlordCombatRuleLibrary::IsValidDemoMasteryId(FName MasteryId)
{
	return FindMappingEntry(MasteryId) != nullptr;
}

ETainlordArchetype UTainlordCombatRuleLibrary::GetMasteryArchetype(FName MasteryId)
{
	if (const FMasteryMappingEntry* Entry = FindMappingEntry(MasteryId))
	{
		return Entry->Archetype;
	}
	
	// Safe fallback: Warrior is the most common archetype
	UE_LOG(LogTemp, Warning, TEXT("TainlordCombatRule: Unknown mastery ID '%s' - defaulting to Warrior archetype"), *MasteryId.ToString());
	return ETainlordArchetype::Warrior;
}

ETainlordStatOrientation UTainlordCombatRuleLibrary::GetMasteryStatOrientation(FName MasteryId)
{
	if (const FMasteryMappingEntry* Entry = FindMappingEntry(MasteryId))
	{
		return Entry->StatOrientation;
	}
	
	// Safe fallback: Strength is the most common orientation
	UE_LOG(LogTemp, Warning, TEXT("TainlordCombatRule: Unknown mastery ID '%s' - defaulting to Strength orientation"), *MasteryId.ToString());
	return ETainlordStatOrientation::Strength;
}

// ---------------------------------------------------------------------------
// Existing Combat Rule Functions
// ---------------------------------------------------------------------------

namespace
{
	int32 GetScalingValue(const FTainlordStatBlock& Stats, const ETainlordScalingAttribute ScalingAttribute)
	{
		switch (ScalingAttribute)
		{
		case ETainlordScalingAttribute::Strength:
			return Stats.Strength;
		case ETainlordScalingAttribute::Intelligence:
			return Stats.Intelligence;
		case ETainlordScalingAttribute::Vitality:
			return Stats.Vitality;
		case ETainlordScalingAttribute::Willpower:
			return Stats.Willpower;
		default:
			return 0;
		}
	}

	FTainlordAttackProfile MakeAction(
		const FName ActionId,
		const FText& DisplayName,
		const ETainlordActionType ActionType,
		const ETainlordScalingAttribute ScalingAttribute,
		const ETainlordElement Element,
		const float BasePower,
		const float ScalingMultiplier,
		const float CooldownSeconds,
		const float ResourceCost,
		const float RangeMeters,
		const bool bStarterAction,
		const bool bSignatureAction)
	{
		FTainlordAttackProfile Profile;
		Profile.ActionId = ActionId;
		Profile.DisplayName = DisplayName;
		Profile.ActionType = ActionType;
		Profile.ScalingAttribute = ScalingAttribute;
		Profile.Element = Element;
		Profile.BasePower = BasePower;
		Profile.ScalingMultiplier = ScalingMultiplier;
		Profile.CooldownSeconds = CooldownSeconds;
		Profile.ResourceCost = ResourceCost;
		Profile.RangeMeters = RangeMeters;
		Profile.bStarterAction = bStarterAction;
		Profile.bSignatureAction = bSignatureAction;
		return Profile;
	}
}

FTainlordStatBlock UTainlordCombatRuleLibrary::BuildDerivedStats(const FTainlordCharacterState& CharacterState)
{
	FTainlordStatBlock Stats;
	Stats.Strength = 8;
	Stats.Intelligence = 8;
	Stats.Vitality = 8;
	Stats.Willpower = 8;

	switch (CharacterState.Progression.Archetype)
	{
	case ETainlordArchetype::Warrior:
		Stats.Strength += 8;
		Stats.Vitality += 6;
		Stats.Willpower += 2;
		break;
	case ETainlordArchetype::Mage:
		Stats.Intelligence += 9;
		Stats.Willpower += 5;
		Stats.Vitality += 1;
		break;
	case ETainlordArchetype::Cleric:
		Stats.Intelligence += 6;
		Stats.Willpower += 6;
		Stats.Vitality += 3;
		break;
	default:
		break;
	}

	switch (CharacterState.Progression.StatOrientation)
	{
	case ETainlordStatOrientation::Strength:
		Stats.Strength += 4;
		Stats.Vitality += 2;
		break;
	case ETainlordStatOrientation::Intelligence:
		Stats.Intelligence += 4;
		Stats.Willpower += 2;
		break;
	case ETainlordStatOrientation::Hybrid:
		Stats.Strength += 2;
		Stats.Intelligence += 2;
		Stats.Willpower += 1;
		break;
	}

	Stats.AttackPower = (Stats.Strength * 3) + Stats.Vitality;
	Stats.SpellPower = (Stats.Intelligence * 3) + Stats.Willpower;
	Stats.Defense = (Stats.Vitality * 2) + FMath::FloorToInt(Stats.Willpower * 0.5f);
	return Stats;
}

FTainlordCombatResources UTainlordCombatRuleLibrary::BuildCombatResources(const FTainlordCharacterState& CharacterState)
{
	const FTainlordStatBlock Stats = BuildDerivedStats(CharacterState);

	FTainlordCombatResources Resources;
	Resources.MaxHealth = 100 + (Stats.Vitality * 12) + (Stats.Defense * 2);
	Resources.MaxMana = 60 + (Stats.Intelligence * 6) + (Stats.Willpower * 4);
	Resources.MaxStamina = 100 + (Stats.Strength * 5) + (Stats.Willpower * 2);
	Resources.CurrentHealth = Resources.MaxHealth;
	Resources.CurrentMana = Resources.MaxMana;
	Resources.CurrentStamina = Resources.MaxStamina;
	return Resources;
}

FTainlordCombatBudget UTainlordCombatRuleLibrary::GetStarterCombatBudget(const ETainlordArchetype Archetype)
{
	FTainlordCombatBudget Budget;
	Budget.Archetype = Archetype;

	switch (Archetype)
	{
	case ETainlordArchetype::Warrior:
		Budget.StarterActions = {
			MakeAction("warrior_basic_slash", NSLOCTEXT("Tainlord", "WarriorBasicSlash", "Basic Slash"), ETainlordActionType::BasicAttack, ETainlordScalingAttribute::Strength, ETainlordElement::Earth, 18.0f, 1.1f, 0.0f, 0.0f, 2.2f, true, false),
			MakeAction("warrior_guard_break", NSLOCTEXT("Tainlord", "WarriorGuardBreak", "Guard Break"), ETainlordActionType::Skill, ETainlordScalingAttribute::Strength, ETainlordElement::Earth, 28.0f, 1.35f, 7.0f, 15.0f, 2.5f, true, true),
			MakeAction("warrior_rally", NSLOCTEXT("Tainlord", "WarriorRally", "Rallying Cry"), ETainlordActionType::Support, ETainlordScalingAttribute::Vitality, ETainlordElement::Fire, 12.0f, 0.8f, 10.0f, 20.0f, 5.0f, true, false)
		};
		break;
	case ETainlordArchetype::Mage:
		Budget.StarterActions = {
			MakeAction("mage_arc_bolt", NSLOCTEXT("Tainlord", "MageArcBolt", "Arc Bolt"), ETainlordActionType::BasicAttack, ETainlordScalingAttribute::Intelligence, ETainlordElement::Fire, 20.0f, 1.15f, 0.0f, 0.0f, 12.0f, true, false),
			MakeAction("mage_flame_orb", NSLOCTEXT("Tainlord", "MageFlameOrb", "Flame Orb"), ETainlordActionType::Skill, ETainlordScalingAttribute::Intelligence, ETainlordElement::Fire, 32.0f, 1.45f, 8.0f, 22.0f, 14.0f, true, true),
			MakeAction("mage_frost_step", NSLOCTEXT("Tainlord", "MageFrostStep", "Frost Step"), ETainlordActionType::Skill, ETainlordScalingAttribute::Willpower, ETainlordElement::Water, 14.0f, 0.9f, 10.0f, 18.0f, 8.0f, true, false)
		};
		break;
	case ETainlordArchetype::Cleric:
		Budget.StarterActions = {
			MakeAction("cleric_mace_strike", NSLOCTEXT("Tainlord", "ClericMaceStrike", "Mace Strike"), ETainlordActionType::BasicAttack, ETainlordScalingAttribute::Intelligence, ETainlordElement::Earth, 16.0f, 1.0f, 0.0f, 0.0f, 2.0f, true, false),
			MakeAction("cleric_restoration", NSLOCTEXT("Tainlord", "ClericRestoration", "Restoration"), ETainlordActionType::Support, ETainlordScalingAttribute::Willpower, ETainlordElement::Water, 30.0f, 1.3f, 9.0f, 24.0f, 8.0f, true, true),
			MakeAction("cleric_purge", NSLOCTEXT("Tainlord", "ClericPurge", "Purge"), ETainlordActionType::Skill, ETainlordScalingAttribute::Intelligence, ETainlordElement::Water, 18.0f, 1.1f, 11.0f, 20.0f, 10.0f, true, false)
		};
		break;
	default:
		break;
	}

	Budget.StarterSkillCount = Budget.StarterActions.Num();
	return Budget;
}

FTainlordCombatResolution UTainlordCombatRuleLibrary::ResolveActionMagnitude(const FTainlordStatBlock& AttackerStats, const FTainlordAttackProfile& ActionProfile, const int32 TargetDefense)
{
	const int32 ScalingValue = GetScalingValue(AttackerStats, ActionProfile.ScalingAttribute);
	const int32 AttackPower = (ActionProfile.ActionType == ETainlordActionType::BasicAttack)
		? AttackerStats.AttackPower
		: AttackerStats.SpellPower;
	const float RawMagnitude = ActionProfile.BasePower + (ScalingValue * ActionProfile.ScalingMultiplier) + (AttackPower * 0.35f);
	const int32 MitigatedMagnitude = FMath::Max(1, FMath::RoundToInt(RawMagnitude) - TargetDefense);

	FTainlordCombatResolution Resolution;
	Resolution.ActionId = ActionProfile.ActionId;
	Resolution.bIsHealing = (ActionProfile.ActionType == ETainlordActionType::Support);
	Resolution.FinalMagnitude = Resolution.bIsHealing ? FMath::RoundToInt(RawMagnitude) : MitigatedMagnitude;
	return Resolution;
}
