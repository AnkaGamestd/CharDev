// Copyright Kingdawn. All Rights Reserved.
// Mastery-level identity types for the Kingdawn progression system.
//
// This file is the authoritative source of truth for mastery identity.
// It sits at the bottom of the mastery dependency chain:
//
//   KDMasteryTypes  (this file)
//     -> KDSkillDefinition
//       -> KDMasteryDefinition
//         -> KDMasterySlot
//           -> KDMasteryComponent
//             -> readers (ASC, Hotbar, Save/Load, UI)
//
// Rules:
// - This file must have ZERO dependencies on other KD types.
// - KDSkillDefinition must NOT reference KDMasteryDefinition.
// - KDMasteryDefinition MAY reference KDSkillDefinition.
// - KDMasteryComponent is state + unlock calculation only.
//
// Migration note:
// EKDCombatClass / EKDCombatBranch in KDBranchTypes.h are the legacy
// equivalents of these enums. New mastery-system code must include
// KDMasteryTypes.h directly and use these enums. KDBranchTypes will
// be retired once all consumers have migrated.

#pragma once

#include "CoreMinimal.h"
#include "KDMasteryTypes.generated.h"

/**
 * Mastery class family.
 *
 * Represents the broad archetype a character's mastery falls under.
 * This is the mastery-layer concept; it does NOT imply equipped
 * weapons, granted abilities, or hotbar layout. Those are separate
 * runtime layers.
 *
 * Replaces: EKDCombatClass (KDBranchTypes.h) for mastery identity.
 */
UENUM(BlueprintType)
enum class EKDMasteryClass : uint8
{
	None       UMETA(DisplayName = "None"),
	Warrior    UMETA(DisplayName = "Warrior"),
	Mage       UMETA(DisplayName = "Mage"),
	Rogue      UMETA(DisplayName = "Rogue"),
	Archer     UMETA(DisplayName = "Archer"),
	Support    UMETA(DisplayName = "Support")
};

/**
 * Mastery branch (specialization line).
 *
 * Represents a specific mastery tree within a class family.
 * A character may have multiple mastery branches active; the
 * mastery component tracks which ones are unlocked and how far
 * each has been progressed.
 *
 * Replaces: EKDCombatBranch (KDBranchTypes.h) for mastery identity.
 */
UENUM(BlueprintType)
enum class EKDMasteryBranch : uint8
{
	None              UMETA(DisplayName = "None"),

	// Warrior branches
	BladeShield       UMETA(DisplayName = "Blade & Shield"),
	TwoHandedSword    UMETA(DisplayName = "Two-Handed Sword"),
	Hammer            UMETA(DisplayName = "Hammer"),
	DualAxe           UMETA(DisplayName = "Dual Axe"),
	Glaive            UMETA(DisplayName = "Glaive"),
	SpearScythe       UMETA(DisplayName = "Spear/Scythe"),

	// Mage branches
	Wizard            UMETA(DisplayName = "Wizard"),
	Warlock           UMETA(DisplayName = "Warlock"),
	Necromancer       UMETA(DisplayName = "Necromancer"),

	// Support branches
	Cleric            UMETA(DisplayName = "Cleric"),
	Buffer            UMETA(DisplayName = "Buffer"),

	// Rogue branches
	Dagger            UMETA(DisplayName = "Dagger"),

	// Archer branches
	Archer            UMETA(DisplayName = "Archer")
};

/**
 * Mastery tier within a branch tree.
 *
 * Skills in a mastery branch are gated by tier. A character must
 * spend enough mastery points in a branch to reach higher tiers.
 * This is a mastery-system concept; the ability system and hotbar
 * do not care about tiers.
 *
 * Replaces: EKDSkillTier (KDAbilityDefinition.h) for mastery gating.
 */
UENUM(BlueprintType)
enum class EKDMasteryTier : uint8
{
	Tier1    UMETA(DisplayName = "Tier 1 - Novice"),
	Tier2    UMETA(DisplayName = "Tier 2 - Adept"),
	Tier3    UMETA(DisplayName = "Tier 3 - Expert"),
	Tier4    UMETA(DisplayName = "Tier 4 - Master")
};

/**
 * Lightweight mastery branch identity pair.
 *
 * Used to identify a character's mastery position without pulling
 * in any runtime state, ability references, or component dependencies.
 * Suitable for data assets, save data, and queries.
 */
USTRUCT(BlueprintType)
struct KINGDAWNCOMBAT_API FKDMasteryIdentity
{
	GENERATED_BODY()

	/** The class family this mastery belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mastery")
	EKDMasteryClass MasteryClass;

	/** The specific branch within the class family. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mastery")
	EKDMasteryBranch MasteryBranch;

	FKDMasteryIdentity()
		: MasteryClass(EKDMasteryClass::None)
		, MasteryBranch(EKDMasteryBranch::None)
	{
	}

	FKDMasteryIdentity(EKDMasteryClass InClass, EKDMasteryBranch InBranch)
		: MasteryClass(InClass)
		, MasteryBranch(InBranch)
	{
	}

	bool IsValid() const
	{
		return MasteryClass != EKDMasteryClass::None && MasteryBranch != EKDMasteryBranch::None;
	}

	bool operator==(const FKDMasteryIdentity& Other) const
	{
		return MasteryClass == Other.MasteryClass && MasteryBranch == Other.MasteryBranch;
	}

	bool operator!=(const FKDMasteryIdentity& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FKDMasteryIdentity& Identity)
	{
		return HashCombine(GetTypeHash(Identity.MasteryClass), GetTypeHash(Identity.MasteryBranch));
	}
};
