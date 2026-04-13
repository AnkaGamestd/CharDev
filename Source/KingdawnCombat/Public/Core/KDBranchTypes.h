// Copyright Kingdawn. All Rights Reserved.
// Branch/class/weapon compatibility types for skill system.
//
// DEPRECATION NOTICE (Phase 1 mastery migration):
// This file is the LEGACY identity authority for class/branch/weapon enums.
// The new authority is KDMasteryTypes.h which defines EKDMasteryClass,
// EKDMasteryBranch, EKDMasteryTier, and FKDMasteryIdentity.
//
// Migration status:
// - KDMasteryTypes.h is created and available for new code.
// - This file CANNOT be removed yet because 5+ headers and 36+ runtime
//   sites still depend on EKDCombatClass, EKDCombatBranch, EKDWeaponType,
//   and FKDSkillCompatibility.
// - New mastery-system code MUST include KDMasteryTypes.h directly.
// - Do NOT add new identity enums here. Add them to KDMasteryTypes.h.
// - FKDSkillCompatibility and EKDWeaponType will remain here until a
//   separate weapon-type migration is performed (they are not mastery identity).
//
// See: plans/mastery-migration.md for the full migration roadmap.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/KDMasteryTypes.h"
#include "KDBranchTypes.generated.h"

/**
 * Combat class family branches for Kingdawn.
 * These represent broad class categories (e.g., Warrior, Mage, Rogue).
 */
UENUM(BlueprintType)
enum class EKDCombatClass : uint8
{
	None UMETA(DisplayName = "None"),
	Warrior UMETA(DisplayName = "Warrior"),
	Mage UMETA(DisplayName = "Mage"),
	Rogue UMETA(DisplayName = "Rogue"),
	Archer UMETA(DisplayName = "Archer"),
	Support UMETA(DisplayName = "Support")
};

/**
 * Specific weapon/branch lines within a class family.
 * A skill may be compatible with one or more branches.
 */
UENUM(BlueprintType)
enum class EKDCombatBranch : uint8
{
	None UMETA(DisplayName = "None"),
	
	// Warrior branches
	BladeShield UMETA(DisplayName = "Blade & Shield"),
	TwoHandedSword UMETA(DisplayName = "Two-Handed Sword"),
	Hammer UMETA(DisplayName = "Hammer"),
	DualAxe UMETA(DisplayName = "Dual Axe"),
	Glaive UMETA(DisplayName = "Glaive"),
	SpearScythe UMETA(DisplayName = "Spear/Scythe"),
	
	// Mage branches
	Wizard UMETA(DisplayName = "Wizard"),
	Warlock UMETA(DisplayName = "Warlock"),
	Necromancer UMETA(DisplayName = "Necromancer"),
	Cleric UMETA(DisplayName = "Cleric"),
	Buffer UMETA(DisplayName = "Buffer"),
	
	// Rogue branches
	Dagger UMETA(DisplayName = "Dagger"),
	
	// Archer branches
	Archer UMETA(DisplayName = "Archer")
};

/**
 * Weapon type enum for equipment-based skill gating.
 * Skills may require specific weapon types to be usable.
 */
UENUM(BlueprintType)
enum class EKDWeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	OneHandedSword UMETA(DisplayName = "One-Handed Sword"),
	Shield UMETA(DisplayName = "Shield"),
	TwoHandedSword UMETA(DisplayName = "Two-Handed Sword"),
	Hammer UMETA(DisplayName = "Hammer"),
	Axe UMETA(DisplayName = "Axe"),
	DualAxe UMETA(DisplayName = "Dual Axe"),
	Glaive UMETA(DisplayName = "Glaive"),
	Spear UMETA(DisplayName = "Spear"),
	Scythe UMETA(DisplayName = "Scythe"),
	Dagger UMETA(DisplayName = "Dagger"),
	Staff UMETA(DisplayName = "Staff"),
	Wand UMETA(DisplayName = "Wand"),
	Bow UMETA(DisplayName = "Bow"),
	Crossbow UMETA(DisplayName = "Crossbow")
};

/**
 * Lightweight skill compatibility descriptor.
 * Used by KDAbilityDefinition to express which classes/branches/weapons can use this skill.
 *
 * Storage authority (dual-storage bridge period):
 *
 * PRIMARY (new code writes here):
 *   RequiredMasteryClass  -> EKDMasteryClass
 *   AllowedMasteryBranches -> TArray<EKDMasteryBranch>
 *
 * MIRROR (auto-populated by SyncLegacyFromMastery()):
 *   RequiredClass          -> EKDCombatClass
 *   AllowedBranches        -> TArray<EKDCombatBranch>
 *
 * INDEPENDENT (not part of mastery identity):
 *   RequiredWeapons        -> TArray<EKDWeaponType>
 *
 * New code should write to the mastery fields and call SyncLegacyFromMastery().
 * Legacy readers continue to work unmodified because the mirror fields stay populated.
 *
 * To complete migration: remove legacy fields once all 49+ call sites use mastery API.
 */
USTRUCT(BlueprintType)
struct FKDSkillCompatibility
{
	GENERATED_BODY()

	// --- Mastery Storage (PRIMARY authority) ---

	/**
	 * Primary mastery class required for this skill.
	 * If None, skill is available to all classes.
	 * NEW CODE should write this field instead of RequiredClass.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility|Mastery")
	EKDMasteryClass RequiredMasteryClass;

	/**
	 * Allowed mastery branches within the class family.
	 * Empty array = all branches in RequiredMasteryClass can use this skill.
	 * NEW CODE should write this field instead of AllowedBranches.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility|Mastery")
	TArray<EKDMasteryBranch> AllowedMasteryBranches;

	// --- Legacy Storage (MIRROR - auto-populated by SyncLegacyFromMastery()) ---

	/**
	 * LEGACY MIRROR: Primary class family required for this skill.
	 * Auto-populated by SyncLegacyFromMastery(). Do not write directly in new code.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compatibility|Legacy")
	EKDCombatClass RequiredClass;

	/**
	 * LEGACY MIRROR: Allowed branches within the class family.
	 * Auto-populated by SyncLegacyFromMastery(). Do not write directly in new code.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compatibility|Legacy")
	TArray<EKDCombatBranch> AllowedBranches;

	// --- Weapon Storage (INDEPENDENT - not part of mastery identity) ---

	/**
	 * Required weapon types to use this skill.
	 * Empty array = no weapon requirement (e.g., magic skills).
	 * Use this for weapon-locked skills (e.g., "requires Dagger").
	 * Weapon identity is separate from mastery identity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
	TArray<EKDWeaponType> RequiredWeapons;

	FKDSkillCompatibility()
		: RequiredMasteryClass(EKDMasteryClass::None)
		, RequiredClass(EKDCombatClass::None)
	{
	}

	// --- Sync ---

	/**
	 * Populate legacy mirror fields from mastery primary fields.
	 * Call this after writing to RequiredMasteryClass / AllowedMasteryBranches.
	 * Legacy readers (IsCompatible, GetCompatibilityTags) will then see correct values.
	 */
	void SyncLegacyFromMastery();

	/**
	 * Populate mastery primary fields from legacy mirror fields.
	 * Use this for one-time migration of existing data assets / code-created defs
	 * that still write to RequiredClass / AllowedBranches.
	 */
	void SyncMasteryFromLegacy();

	// --- Legacy API (EKDCombatClass/EKDCombatBranch) ---

	/**
	 * Check if this skill is compatible with the given class/branch/weapon setup.
	 * Reads from legacy mirror fields. No behavioral change from before migration.
	 */
	bool IsCompatible(EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon) const;

	/**
	 * Get gameplay tags for this compatibility (for tag-based checks).
	 * Reads from legacy mirror fields. Returns tags like KD.Class.Warrior, KD.Branch.BladeShield.
	 */
	FGameplayTagContainer GetCompatibilityTags() const;

	// --- Mastery-Aware API (FKDMasteryIdentity) ---

	/**
	 * Check compatibility using mastery identity.
	 * Reads from mastery primary fields directly.
	 * This is the preferred API for new mastery-system code.
	 *
	 * @param MasteryId The character's mastery identity (class + branch)
	 * @param Weapon The character's equipped weapon type
	 * @return True if the skill can be used by this configuration
	 */
	bool IsCompatibleWithMastery(const struct FKDMasteryIdentity& MasteryId, EKDWeaponType Weapon) const;
};

// --- Legacy-to-Mastery Conversion Helpers ---

/**
 * Convert legacy EKDCombatClass to mastery EKDMasteryClass.
 * Values have 1:1 mapping. Returns None for unrecognized values.
 */
KINGDAWNCOMBAT_API EKDMasteryClass ConvertToMasteryClass(EKDCombatClass LegacyClass);

/**
 * Convert legacy EKDCombatBranch to mastery EKDMasteryBranch.
 * Values have 1:1 mapping. Returns None for unrecognized values.
 */
KINGDAWNCOMBAT_API EKDMasteryBranch ConvertToMasteryBranch(EKDCombatBranch LegacyBranch);

/**
 * Convert mastery EKDMasteryClass back to legacy EKDCombatClass.
 * Used by mastery-aware overloads that delegate to legacy storage.
 */
KINGDAWNCOMBAT_API EKDCombatClass ConvertToLegacyClass(EKDMasteryClass MasteryClass);

/**
 * Convert mastery EKDMasteryBranch back to legacy EKDCombatBranch.
 * Used by mastery-aware overloads that delegate to legacy storage.
 */
KINGDAWNCOMBAT_API EKDCombatBranch ConvertToLegacyBranch(EKDMasteryBranch MasteryBranch);
