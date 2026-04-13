// Copyright Kingdawn. All Rights Reserved.
// Clean skill definition for the mastery system.
//
// This is a NEW file, separate from the legacy KDAbilityDefinition.
// It has NO backward compatibility burden and follows the new
// mastery architecture rules strictly.
//
// Hard rule: This file must NOT reference KDMasteryDefinition.
// Dependency is one-way: KDMasteryDefinition may reference this file.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/KDMasteryTypes.h"
#include "KDSkillDefinition.generated.h"

class UGameplayAbility;
class UTexture2D;

/**
 * Clean skill definition for the mastery system.
 *
 * This defines a single skill that can be unlocked in a mastery tree.
 * It does NOT contain:
 * - Mastery tree position/prerequisites (that's KDMasterySlot)
 * - Compatibility checks (that's mastery-based gating)
 * - Montage/cost/range data (that's the ability class itself)
 *
 * It DOES contain:
 * - Skill identity (tag + display name)
 * - Mastery tier requirement
 * - Mastery point unlock cost
 * - Runtime ability class to grant when unlocked
 * - UI presentation data (icon, description)
 */
UCLASS(BlueprintType)
class KINGDAWNCOMBAT_API UKDSkillDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UKDSkillDefinition();

	// --- Identity ---

	/**
	 * Unique skill identity tag (e.g., KD.Skill.BladeShield.HeavyStrike).
	 * This is the runtime handle for this skill.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FGameplayTag SkillTag;

	/**
	 * Display name shown in UI.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FText DisplayName;

	/**
	 * Description shown in tooltips.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FText Description;

	/**
	 * Icon shown in UI (skill tree, hotbar, etc.).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	TObjectPtr<UTexture2D> Icon;

	// --- Mastery Gating ---

	/**
	 * Minimum mastery tier required to unlock this skill.
	 * A character must reach this tier in the parent mastery branch.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Gating")
	EKDMasteryTier RequiredTier;

	/**
	 * Mastery points required to unlock this skill.
	 * Spent when the skill is unlocked from the mastery tree.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Gating")
	int32 MasteryPointCost;

	// --- Runtime Ability ---

	/**
	 * The GameplayAbility class to grant when this skill is unlocked.
	 * Soft reference to avoid loading all abilities at once.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Runtime")
	TSoftClassPtr<UGameplayAbility> AbilityClass;

	// --- Query Helpers ---

	/**
	 * Check if this skill definition is valid for runtime use.
	 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsValid() const
	{
		return SkillTag.IsValid() && !AbilityClass.IsNull();
	}
};
