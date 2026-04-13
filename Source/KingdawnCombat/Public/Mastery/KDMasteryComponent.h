// Copyright Kingdawn. All Rights Reserved.
// Mastery state component: owns mastery state and unlock calculation.
//
// Dependency chain:
//   KDMasteryTypes -> KDSkillDefinition -> KDMasterySlot
//     -> KDMasteryDefinition -> KDMasteryComponent (this)
//       -> readers: KDAbilitySystemComponent, KDHotbarComponent, save/load, UI
//
// Arch.md hard rules enforced here:
// - State + unlock calculation ONLY
// - Does NOT grant or revoke abilities
// - Does NOT place skills on the hotbar
// - Does NOT save or load itself
// - Does NOT know GAS tags, montages, hotbar slots, or input keys
// - Must NOT include any other component class

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Core/KDMasteryTypes.h"
#include "KDMasteryComponent.generated.h"

class UKDMasteryDefinition;
class UKDSkillDefinition;

/**
 * Delegate broadcast when a skill is unlocked.
 * Listeners (ASC, Hotbar, UI) react to this event.
 * The mastery component does NOT call or manage those listeners.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FKDOnSkillUnlocked,
	FGameplayTag, SkillTag,
	FKDMasteryIdentity, MasteryIdentity
);

/**
 * Delegate broadcast when a mastery is activated or deactivated.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FKDOnMasteryChanged,
	FKDMasteryIdentity, MasteryIdentity
);

/**
 * Mastery state component.
 *
 * Owns:
 * - Active mastery list
 * - Spent mastery points per branch
 * - Unlocked skill list per branch
 *
 * Provides:
 * - Unlock eligibility queries (IsSkillUnlocked, CanUnlockSkill, etc.)
 * - State mutation (TryUnlockSkill, ActivateMastery, etc.)
 * - Events for downstream systems to listen to
 *
 * Does NOT provide:
 * - Ability granting/revoking (KDAbilitySystemComponent's job)
 * - Hotbar slot placement (KDHotbarComponent's job)
 * - Save/load serialization (separate system's job)
 * - GAS tags, montages, input keys (those are ability-layer concerns)
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDMasteryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDMasteryComponent();

	// ========================================================================
	// Query API (read-only state)
	// ========================================================================

	/**
	 * Check if a specific skill is unlocked (across all active masteries).
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	bool IsSkillUnlocked(FGameplayTag SkillTag) const;

	/**
	 * Get all unlocked skill tags for a specific mastery branch.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	TArray<FGameplayTag> GetUnlockedSkills(FKDMasteryIdentity Identity) const;

	/**
	 * Get all unlocked skill tags across all active masteries.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	TArray<FGameplayTag> GetAllUnlockedSkills() const;

	/**
	 * Find the skill definition asset for a given skill tag.
	 * Searches all active mastery definitions' slots for a matching skill.
	 * Returns nullptr if no definition is found or the skill asset cannot be loaded.
	 *
	 * This is a query-only API. The mastery component does not interpret
	 * the returned definition's ability class or other runtime fields.
	 * That is the reader's responsibility (e.g., KDAbilitySystemComponent).
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	UKDSkillDefinition* FindSkillDefinition(FGameplayTag SkillTag) const;

	/**
	 * Get mastery points spent in a specific branch.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	int32 GetSpentPoints(FKDMasteryIdentity Identity) const;

	/**
	 * Get the list of currently active mastery identities.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	const TArray<FKDMasteryIdentity>& GetActiveMasteries() const { return ActiveMasteries; }

	/**
	 * Check if a mastery branch is currently active.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	bool IsMasteryActive(FKDMasteryIdentity Identity) const;

	/**
	 * Check if a skill can be unlocked right now.
	 * Validates: mastery active, tier reached, points available, prerequisites met.
	 * Does NOT check ability-layer concerns (GAS state, etc.).
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	bool CanUnlockSkill(FGameplayTag SkillTag, FKDMasteryIdentity Identity) const;

	// ========================================================================
	// Mutation API (state changes)
	// ========================================================================

	/**
	 * Activate a mastery branch for this character.
	 * Publishes OnMasteryChanged event. Does NOT grant abilities.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mastery|Mutation")
	void ActivateMastery(FKDMasteryIdentity Identity, UKDMasteryDefinition* Definition);

	/**
	 * Deactivate a mastery branch. Unlocked skills remain recorded.
	 * Publishes OnMasteryChanged event. Does NOT revoke abilities.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mastery|Mutation")
	void DeactivateMastery(FKDMasteryIdentity Identity);

	/**
	 * Try to unlock a skill in a specific mastery branch.
	 * Returns true if the skill was successfully unlocked.
	 * Returns false if any validation fails.
	 * Publishes OnSkillUnlocked event on success.
	 * Does NOT grant the corresponding ability.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mastery|Mutation")
	bool TryUnlockSkill(FGameplayTag SkillTag, FKDMasteryIdentity Identity);

	/**
	 * Add mastery points to a specific branch.
	 * These points become available for spending on skill unlocks.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mastery|Mutation")
	void AddMasteryPoints(FKDMasteryIdentity Identity, int32 Points);

	/**
	 * Get available (unspent) mastery points for a branch.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	int32 GetAvailablePoints(FKDMasteryIdentity Identity) const;

	// ========================================================================
	// State restoration (for external save/load systems)
	// ========================================================================

	/**
	 * Get total mastery points earned for a specific branch (spent + available).
	 * External save/load systems may use this to serialize point totals.
	 */
	UFUNCTION(BlueprintPure, Category = "Mastery|Query")
	int32 GetTotalPoints(FKDMasteryIdentity Identity) const;

	/**
	 * Restore mastery branch state from external data.
	 * This is a low-level state restoration API for save/load systems.
	 * It does NOT validate prerequisites or trigger events.
	 * It does NOT grant abilities or update hotbar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mastery|Restoration")
	void RestoreMasteryState(FKDMasteryIdentity Identity, const TArray<FGameplayTag>& UnlockedSkills, int32 TotalPoints, int32 SpentPoints);

	// ========================================================================
	// Events (published, not managed)
	// ========================================================================

	/**
	 * Broadcast when a skill is unlocked.
	 * Listeners: KDAbilitySystemComponent (to grant ability),
	 *            KDHotbarComponent (to update slot availability), UI
	 */
	UPROPERTY(BlueprintAssignable, Category = "Mastery|Events")
	FKDOnSkillUnlocked OnSkillUnlocked;

	/**
	 * Broadcast when a mastery is activated or deactivated.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Mastery|Events")
	FKDOnMasteryChanged OnMasteryActivated;

	UPROPERTY(BlueprintAssignable, Category = "Mastery|Events")
	FKDOnMasteryChanged OnMasteryDeactivated;

private:

	// --- Internal state ---

	/** Currently active mastery branches. */
	UPROPERTY()
	TArray<FKDMasteryIdentity> ActiveMasteries;

	/** Mastery definitions for active masteries (runtime references). */
	UPROPERTY()
	TMap<FKDMasteryIdentity, TObjectPtr<UKDMasteryDefinition>> MasteryDefinitions;

	/** Unlocked skill tags per mastery identity. */
	TMap<FKDMasteryIdentity, TArray<FGameplayTag>> UnlockedSkillsMap;

	/** Mastery points spent per mastery identity. */
	TMap<FKDMasteryIdentity, int32> SpentPointsMap;

	/** Total mastery points earned per mastery identity (spent + available). */
	TMap<FKDMasteryIdentity, int32> TotalPointsMap;
};
