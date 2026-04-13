// Copyright Kingdawn. All Rights Reserved.
// Mastery branch definition: a complete skill tree for a mastery branch.
//
// Dependency chain:
//   KDMasteryTypes -> KDSkillDefinition -> KDMasterySlot
//     -> KDMasteryDefinition (this) -> KDMasteryComponent
//
// Hard rule: This file MAY reference KDSkillDefinition.
// This is the ONE-WAY dependency direction enforced by Arch.md.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/KDMasteryTypes.h"
#include "Mastery/KDMasterySlot.h"
#include "KDMasteryDefinition.generated.h"

/**
 * Master definition for a complete mastery branch tree.
 *
 * Designers author one UKDMasteryDefinition per mastery branch
 * (e.g., "BladeShield", "Wizard"). It contains:
 * - Identity (class + branch)
 * - All skill slots in the tree
 * - Tier unlock thresholds (points required to reach each tier)
 *
 * This is a data asset that lives in the Content Browser.
 * At runtime, KDMasteryComponent queries these definitions to
 * calculate unlock eligibility.
 */
UCLASS(BlueprintType)
class KINGDAWNCOMBAT_API UKDMasteryDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UKDMasteryDefinition();

	// --- Identity ---

	/**
	 * Identity of this mastery branch (class + branch pair).
	 * Must be unique across all mastery definitions.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mastery|Identity")
	FKDMasteryIdentity Identity;

	/**
	 * Display name shown in UI (e.g., "Blade & Shield Mastery").
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mastery|Identity")
	FText DisplayName;

	/**
	 * Description shown in mastery selection UI.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mastery|Identity")
	FText Description;

	/**
	 * Icon shown in mastery selection UI.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mastery|Identity")
	TObjectPtr<UTexture2D> Icon;

	// --- Skill Tree ---

	/**
	 * All skill slots in this mastery tree.
	 * Ordered array (index is presentation-only; identity comes from SlotTag).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mastery|Tree")
	TArray<FKDMasterySlot> Slots;

	/**
	 * Mastery points required to unlock each tier.
	 * Key = tier, Value = cumulative points spent in this branch.
	 * Example: {Tier1: 0, Tier2: 5, Tier3: 15, Tier4: 30}
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mastery|Progression")
	TMap<EKDMasteryTier, int32> TierUnlockThresholds;

	// --- Query Helpers ---

	/**
	 * Find a slot by its tag (C++ only, returns pointer).
	 * Returns nullptr if not found.
	 */
	const FKDMasterySlot* FindSlotByTag(FGameplayTag SlotTag) const;

	/**
	 * Blueprint-safe slot lookup. Returns true if found.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery")
	bool GetSlotByTag(FGameplayTag SlotTag, FKDMasterySlot& OutSlot) const;

	/**
	 * Check if the given tier is unlocked with the provided spent points.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery")
	bool IsTierUnlocked(EKDMasteryTier Tier, int32 SpentPoints) const;

	/**
	 * Get the highest tier unlocked with the given spent points.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery")
	EKDMasteryTier GetHighestUnlockedTier(int32 SpentPoints) const;
};
