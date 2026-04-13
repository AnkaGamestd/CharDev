// Copyright Kingdawn. All Rights Reserved.
// Mastery tree slot: a position in a mastery branch tree.
//
// Dependency chain:
//   KDMasteryTypes -> KDSkillDefinition -> KDMasterySlot (this)
//     -> KDMasteryDefinition -> KDMasteryComponent
//
// This is a USTRUCT, not a data asset, because slots are authored
// inline within KDMasteryDefinition. They describe skill positions
// and prerequisite relationships within a single mastery tree.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KDMasterySlot.generated.h"

class UKDSkillDefinition;

/**
 * A single slot in a mastery branch tree.
 *
 * Each slot references a KDSkillDefinition and defines:
 * - Position identity (tag-based, not index-based)
 * - Prerequisite slots (must be unlocked before this one)
 * - UI position in the tree layout
 *
 * Slots are authored inline within UKDMasteryDefinition.
 * They do NOT own the skill; they reference it via soft pointer.
 */
USTRUCT(BlueprintType)
struct KINGDAWNCOMBAT_API FKDMasterySlot
{
	GENERATED_BODY()

	/**
	 * Unique tag identifying this slot within its mastery tree.
	 * Used for prerequisite references and unlock tracking.
	 * Example: KD.Mastery.BladeShield.Slot.HeavyStrike
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MasterySlot")
	FGameplayTag SlotTag;

	/**
	 * The skill definition this slot unlocks.
	 * Soft reference to avoid loading all skills at startup.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MasterySlot")
	TSoftObjectPtr<UKDSkillDefinition> Skill;

	/**
	 * Prerequisite slot tags that must be unlocked before this slot.
	 * Empty = root slot (no prerequisites).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MasterySlot")
	TArray<FGameplayTag> PrerequisiteSlots;

	/**
	 * Position in the mastery tree UI layout.
	 * X = column, Y = row. Used only for presentation.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MasterySlot|UI")
	FVector2D TreePosition;

	FKDMasterySlot()
		: TreePosition(FVector2D::ZeroVector)
	{
	}

	/**
	 * Check if this slot has a valid configuration.
	 */
	bool IsValid() const
	{
		return SlotTag.IsValid() && !Skill.IsNull();
	}
};
