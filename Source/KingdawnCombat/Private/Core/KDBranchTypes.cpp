// Copyright Kingdawn. All Rights Reserved.

#include "Core/KDBranchTypes.h"
#include "Core/KDMasteryTypes.h"

// --- FKDSkillCompatibility: Legacy API ---

bool FKDSkillCompatibility::IsCompatible(EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon) const
{
	// If no class requirement, skill is universal
	if (RequiredClass == EKDCombatClass::None)
	{
		// Still check weapon requirement if specified
		if (RequiredWeapons.Num() > 0 && !RequiredWeapons.Contains(Weapon))
		{
			return false;
		}
		return true;
	}

	// Check class match
	if (Class != RequiredClass)
	{
		return false;
	}

	// Check branch restriction (empty = all branches in class are allowed)
	if (AllowedBranches.Num() > 0 && !AllowedBranches.Contains(Branch))
	{
		return false;
	}

	// Check weapon requirement (empty = no weapon restriction)
	if (RequiredWeapons.Num() > 0 && !RequiredWeapons.Contains(Weapon))
	{
		return false;
	}

	return true;
}

FGameplayTagContainer FKDSkillCompatibility::GetCompatibilityTags() const
{
	FGameplayTagContainer Tags;

	// Add class tag
	if (RequiredClass != EKDCombatClass::None)
	{
		FString ClassTagName = FString::Printf(TEXT("KD.Class.%s"), *UEnum::GetDisplayValueAsText(RequiredClass).ToString());
		FGameplayTag ClassTag = FGameplayTag::RequestGameplayTag(FName(*ClassTagName), false);
		if (ClassTag.IsValid())
		{
			Tags.AddTag(ClassTag);
		}
	}

	// Add branch tags
	for (EKDCombatBranch Branch : AllowedBranches)
	{
		if (Branch != EKDCombatBranch::None)
		{
			FString BranchTagName = FString::Printf(TEXT("KD.Branch.%s"), *UEnum::GetDisplayValueAsText(Branch).ToString());
			FGameplayTag BranchTag = FGameplayTag::RequestGameplayTag(FName(*BranchTagName), false);
			if (BranchTag.IsValid())
			{
				Tags.AddTag(BranchTag);
			}
		}
	}

	return Tags;
}

// --- FKDSkillCompatibility: Sync Methods ---

void FKDSkillCompatibility::SyncLegacyFromMastery()
{
	// Populate legacy mirror fields from mastery primary fields
	RequiredClass = ConvertToLegacyClass(RequiredMasteryClass);
	
	AllowedBranches.Empty();
	for (EKDMasteryBranch MasteryBranch : AllowedMasteryBranches)
	{
		AllowedBranches.Add(ConvertToLegacyBranch(MasteryBranch));
	}
}

void FKDSkillCompatibility::SyncMasteryFromLegacy()
{
	// Populate mastery primary fields from legacy mirror fields
	RequiredMasteryClass = ConvertToMasteryClass(RequiredClass);
	
	AllowedMasteryBranches.Empty();
	for (EKDCombatBranch LegacyBranch : AllowedBranches)
	{
		AllowedMasteryBranches.Add(ConvertToMasteryBranch(LegacyBranch));
	}
}

// --- FKDSkillCompatibility: Mastery-Aware API ---

bool FKDSkillCompatibility::IsCompatibleWithMastery(const FKDMasteryIdentity& MasteryId, EKDWeaponType Weapon) const
{
	// Read from mastery primary fields directly (no conversion needed)
	
	// If no class requirement, skill is universal
	if (RequiredMasteryClass == EKDMasteryClass::None)
	{
		// Still check weapon requirement if specified
		if (RequiredWeapons.Num() > 0 && !RequiredWeapons.Contains(Weapon))
		{
			return false;
		}
		return true;
	}

	// Check class match
	if (MasteryId.MasteryClass != RequiredMasteryClass)
	{
		return false;
	}

	// Check branch restriction (empty = all branches in class are allowed)
	if (AllowedMasteryBranches.Num() > 0 && !AllowedMasteryBranches.Contains(MasteryId.MasteryBranch))
	{
		return false;
	}

	// Check weapon requirement (empty = no weapon restriction)
	if (RequiredWeapons.Num() > 0 && !RequiredWeapons.Contains(Weapon))
	{
		return false;
	}

	return true;
}

// --- Legacy-to-Mastery Conversion Helpers ---

EKDMasteryClass ConvertToMasteryClass(EKDCombatClass LegacyClass)
{
	switch (LegacyClass)
	{
	case EKDCombatClass::Warrior: return EKDMasteryClass::Warrior;
	case EKDCombatClass::Mage:    return EKDMasteryClass::Mage;
	case EKDCombatClass::Rogue:   return EKDMasteryClass::Rogue;
	case EKDCombatClass::Archer:  return EKDMasteryClass::Archer;
	case EKDCombatClass::Support: return EKDMasteryClass::Support;
	default:                      return EKDMasteryClass::None;
	}
}

EKDMasteryBranch ConvertToMasteryBranch(EKDCombatBranch LegacyBranch)
{
	switch (LegacyBranch)
	{
	case EKDCombatBranch::BladeShield:    return EKDMasteryBranch::BladeShield;
	case EKDCombatBranch::TwoHandedSword: return EKDMasteryBranch::TwoHandedSword;
	case EKDCombatBranch::Hammer:         return EKDMasteryBranch::Hammer;
	case EKDCombatBranch::DualAxe:        return EKDMasteryBranch::DualAxe;
	case EKDCombatBranch::Glaive:         return EKDMasteryBranch::Glaive;
	case EKDCombatBranch::SpearScythe:    return EKDMasteryBranch::SpearScythe;
	case EKDCombatBranch::Wizard:         return EKDMasteryBranch::Wizard;
	case EKDCombatBranch::Warlock:        return EKDMasteryBranch::Warlock;
	case EKDCombatBranch::Necromancer:    return EKDMasteryBranch::Necromancer;
	case EKDCombatBranch::Cleric:         return EKDMasteryBranch::Cleric;
	case EKDCombatBranch::Buffer:         return EKDMasteryBranch::Buffer;
	case EKDCombatBranch::Dagger:         return EKDMasteryBranch::Dagger;
	case EKDCombatBranch::Archer:         return EKDMasteryBranch::Archer;
	default:                              return EKDMasteryBranch::None;
	}
}

EKDCombatClass ConvertToLegacyClass(EKDMasteryClass MasteryClass)
{
	switch (MasteryClass)
	{
	case EKDMasteryClass::Warrior: return EKDCombatClass::Warrior;
	case EKDMasteryClass::Mage:    return EKDCombatClass::Mage;
	case EKDMasteryClass::Rogue:   return EKDCombatClass::Rogue;
	case EKDMasteryClass::Archer:  return EKDCombatClass::Archer;
	case EKDMasteryClass::Support: return EKDCombatClass::Support;
	default:                       return EKDCombatClass::None;
	}
}

EKDCombatBranch ConvertToLegacyBranch(EKDMasteryBranch MasteryBranch)
{
	switch (MasteryBranch)
	{
	case EKDMasteryBranch::BladeShield:    return EKDCombatBranch::BladeShield;
	case EKDMasteryBranch::TwoHandedSword: return EKDCombatBranch::TwoHandedSword;
	case EKDMasteryBranch::Hammer:         return EKDCombatBranch::Hammer;
	case EKDMasteryBranch::DualAxe:        return EKDCombatBranch::DualAxe;
	case EKDMasteryBranch::Glaive:         return EKDCombatBranch::Glaive;
	case EKDMasteryBranch::SpearScythe:    return EKDCombatBranch::SpearScythe;
	case EKDMasteryBranch::Wizard:         return EKDCombatBranch::Wizard;
	case EKDMasteryBranch::Warlock:        return EKDCombatBranch::Warlock;
	case EKDMasteryBranch::Necromancer:    return EKDCombatBranch::Necromancer;
	case EKDMasteryBranch::Cleric:         return EKDCombatBranch::Cleric;
	case EKDMasteryBranch::Buffer:         return EKDCombatBranch::Buffer;
	case EKDMasteryBranch::Dagger:         return EKDCombatBranch::Dagger;
	case EKDMasteryBranch::Archer:         return EKDCombatBranch::Archer;
	default:                               return EKDCombatBranch::None;
	}
}
