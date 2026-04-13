// Copyright Kingdawn. All Rights Reserved.
// Adapted from ActionsSystem/Components/ACFAbilitySystemComponent.h
// Simplified: Removed moveset system, ACFAnimInstance, action buffering, combo counters
// Kept: Ability granting by tag, ability triggering, ability state tracking
//
// Phase 3 integration: read-only mastery query surface added.
// KDAbilitySystemComponent reads KDMasteryComponent to decide runtime
// grant/revoke state. It does NOT own mastery state.

#pragma once

#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KDAbilitySystemComponent.generated.h"

class UKDGameplayAbility;
class UKDMasteryComponent;
class UKDSkillDefinition;

/**
 * Result of a mastery grant reconciliation pass.
 * Reports what was granted, what was revoked, and what failed.
 */
USTRUCT(BlueprintType)
struct FKDMasteryReconciliationResult
{
	GENERATED_BODY()

	/** Skill tags that were missing and have now been granted by mastery reconciliation. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Mastery")
	TArray<FGameplayTag> GrantedSkills;

	/** Skill tags that were previously mastery-granted but are no longer unlocked,
	 * and have been safely revoked. Only abilities tracked in MasteryGrantedSkills
	 * are revoked; build-granted or test-granted abilities are never touched. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Mastery")
	TArray<FGameplayTag> RevokedSkills;

	/** Skill tags that failed to grant (definition missing or ability class invalid). */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Mastery")
	TArray<FGameplayTag> FailedSkills;
};

/** Broadcast when an ability starts. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKDOnAbilityStarted, FGameplayTag, AbilityTag);

/** Broadcast when an ability ends. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKDOnAbilityEnded, FGameplayTag, AbilityTag);

/**
 * Simplified Ability System Component for Kingdawn combat.
 * Extends UAbilitySystemComponent with tag-based ability triggering,
 * ability state tracking, and combo counter support.
 *
 * Removed from ACF version:
 * - Moveset system (SetMovesetActions, MovesetAbilities map)
 * - ACFAnimInstance references
 * - Action buffering (StoreAbilityInBuffer)
 * - ARSStatisticsComponent dependency
 * - ACFActionsSet / ACFAbilitySet complex granting
 */
UCLASS(ClassGroup = (Kingdawn), DisplayName = "KD Ability Component", Blueprintable, meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UKDAbilitySystemComponent();

	// --- Replication ---

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Delegates ---

	/** Broadcast when an ability starts executing. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnAbilityStarted OnAbilityStartedEvent;

	/** Broadcast when an ability finishes executing. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnAbilityEnded OnAbilityFinishedEvent;

	// --- Ability Triggering ---

	/**
	 * Trigger an ability by its gameplay tag.
	 * @param AbilityTag The tag identifying the ability to trigger.
	 * @return True if the ability was successfully triggered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	bool TriggerAbilityByTag(FGameplayTag AbilityTag);

	/**
	 * Send a gameplay event to all active abilities.
	 * @param GameplayEvent The event tag to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void SendGameplayEvent(const FGameplayTag& GameplayEvent);

	// --- Ability Granting ---

	/**
	 * Grant an ability class with optional tag and input binding.
	 * @param AbilityClass The ability class to grant.
	 * @param AbilityTag Tag to identify this ability.
	 * @param InputAction Optional input action to bind.
	 * @return The handle of the granted ability spec.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	FGameplayAbilitySpecHandle GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTag AbilityTag, int32 InputID = 0);

	/**
	 * Remove an ability by its tag.
	 * @param AbilityTag The tag of the ability to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void RemoveAbilityByTag(FGameplayTag AbilityTag);

	// --- Getters ---

	/**
	 * Get the ability instance by tag.
	 * @param AbilityTag The tag to search for.
	 * @return The ability instance, or nullptr if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UGameplayAbility* GetAbilityByTag(const FGameplayTag& AbilityTag) const;

	/**
	 * Get the handle for an ability by tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	FGameplayAbilitySpecHandle GetAbilityHandleByTag(const FGameplayTag& AbilityTag) const;

	/**
	 * Check if currently performing an ability.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	bool IsPerformingAbility() const { return bIsPerformingAbility; }

	/**
	 * Get the tag of the currently active ability.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	FGameplayTag GetCurrentAbilityTag() const { return CurrentAbilityTag; }

	/**
	 * Check if a specific ability is currently active.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	bool IsInAbilityState(FGameplayTag State) const { return CurrentAbilityTag == State; }

	// --- Cooldown State ---

	/**
	 * Check whether a granted ability tag is currently on runtime cooldown.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Cooldown")
	bool IsAbilityTagOnCooldown(FGameplayTag AbilityTag) const;

	/**
	 * Get remaining runtime cooldown in seconds for a granted ability tag.
	 * Returns 0 when the tag is not cooling down.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Cooldown")
	float GetAbilityTagCooldownRemaining(FGameplayTag AbilityTag) const;

	/**
	 * Start or refresh runtime cooldown for a granted ability tag.
	 */
	void StartAbilityTagCooldown(FGameplayTag AbilityTag, float DurationSeconds);

	/**
	 * Remove any runtime cooldown tracking for a granted ability tag.
	 */
	void ClearAbilityTagCooldown(FGameplayTag AbilityTag);

	// --- Combo Counters ---

	/**
	 * Set the combo counter for a given combo tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void SetComboCounter(const FGameplayTag& ComboTag, int32 Value);

	/**
	 * Get the combo counter for a given combo tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	int32 GetComboCount(const FGameplayTag& ComboTag) const;

	/**
	 * Reset the combo counter for a given combo tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void ResetComboCount(const FGameplayTag& ComboTag);

	// --- Ability Lifecycle ---

	/** Called when an ability starts. */
	void NotifyAbilityStarted(UGameplayAbility* Ability);

	/** Called when an ability ends. Override from base class. */
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	// ========================================================================
	// Mastery Integration (read-only queries)
	// ========================================================================

	/**
	 * Get the mastery component on this actor, if present.
	 * Returns nullptr if no mastery component exists.
	 * Used internally by mastery query methods.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Mastery")
	UKDMasteryComponent* GetMasteryComponent() const;

	/**
	 * Check if a skill is unlocked in the mastery system.
	 * Delegates to KDMasteryComponent::IsSkillUnlocked().
	 * Returns false if no mastery component is present.
	 *
	 * This is the primary query the ASC uses to determine whether
	 * a skill's ability should exist in the granted ability set.
	 * Unlock state != granted state; the ASC still owns the grant.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Mastery")
	bool IsMasterySkillUnlocked(FGameplayTag SkillTag) const;

	/**
	 * Get all skill tags that are unlocked across all active masteries.
	 * Delegates to KDMasteryComponent::GetAllUnlockedSkills().
	 * Returns empty array if no mastery component is present.
	 *
	 * Useful for full reconciliation (e.g., on load or respawn)
	 * when the ASC needs to compare granted abilities against
	 * the mastery unlock state.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Mastery")
	TArray<FGameplayTag> GetAllMasteryUnlockedSkills() const;

	// ========================================================================
	// Mastery Grant Reconciliation
	// ========================================================================

	/**
	 * Mastery grant reconciliation with safe revoke.
	 *
	 * Compares mastery-unlocked skills against currently granted abilities.
	 * - Grants missing abilities (skill unlocked in mastery but not yet granted).
	 * - Revokes stale mastery-granted abilities (previously granted by mastery
	 *   reconciliation but no longer unlocked). Only abilities tracked in
	 *   MasteryGrantedSkills are revoked; build-granted or test-granted
	 *   abilities are never touched.
	 * - Reports failed grants (skill definition or ability class missing/unloadable).
	 *
	 * GRANT-SOURCE TRACKING:
	 * MasteryGrantedSkills is a TSet<FGameplayTag> that records which abilities
	 * were granted BY THIS RECONCILIATION PATH. Rule:
	 *   - When reconciliation grants a new ability, the tag is added to MasteryGrantedSkills.
	 *   - When reconciliation revokes a stale ability, the tag is removed from MasteryGrantedSkills.
	 *   - Abilities granted through other paths (build system, test code, Blueprint)
	 *     are NEVER added to MasteryGrantedSkills and are NEVER revoked here.
	 *   - If a skill is already granted when reconciliation runs (e.g., build-granted)
	 *     AND it is also mastery-unlocked, reconciliation does NOT claim ownership.
	 *     It skips the grant and does NOT add it to MasteryGrantedSkills.
	 *     This prevents accidental revocation of build-granted abilities that
	 *     happen to overlap with mastery-unlocked skills.
	 *
	 * AUTHORITY:
	 * Skill tag -> ability class resolution uses KDSkillDefinition::AbilityClass
	 * (the new mastery data path). This does NOT use the legacy KDAbilityDefinition
	 * or KDSkillDefinitionLibrary.
	 *
	 * INTENDED CALL SITES:
	 * - After mastery state is loaded/restored
	 * - After a skill is unlocked (OnSkillUnlocked listener)
	 * - After a mastery is activated/deactivated
	 * - Manual call from test harness or Blueprint
	 *
	 * @return Result struct describing what was granted, what was revoked, and what failed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Mastery")
	FKDMasteryReconciliationResult ReconcileMasteryGrantedAbilities();

	/**
	 * Check if a skill tag was granted by the mastery reconciliation path.
	 * Returns false for abilities granted by build system, test code, or Blueprint.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Mastery")
	bool IsMasteryGrantedSkill(FGameplayTag SkillTag) const;

	/**
	 * Get all skill tags currently tracked as mastery-granted.
	 * This is NOT the same as "all granted abilities" — it is only
	 * the subset granted by ReconcileMasteryGrantedAbilities().
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Mastery")
	const TSet<FGameplayTag>& GetMasteryGrantedSkills() const;

protected:
	virtual void BeginPlay() override;

	// ========================================================================
	// Mastery Event Handlers (called by KDMasteryComponent delegates)
	// ========================================================================

	/** Called when a skill is unlocked in the mastery system. Triggers reconciliation. */
	UFUNCTION()
	void OnMasterySkillUnlocked(FGameplayTag SkillTag, struct FKDMasteryIdentity MasteryIdentity);

	/** Called when a mastery branch is activated. Triggers reconciliation. */
	UFUNCTION()
	void OnMasteryActivated(struct FKDMasteryIdentity MasteryIdentity);

	/** Called when a mastery branch is deactivated. Triggers reconciliation. */
	UFUNCTION()
	void OnMasteryDeactivated(struct FKDMasteryIdentity MasteryIdentity);

private:
	/** Whether an ability is currently being performed. */
	UPROPERTY(Replicated)
	bool bIsPerformingAbility;

	/** Flag to prevent duplicate mastery event binding in BeginPlay. */
	bool bMasteryEventsBound;

	/** Tag of the currently active ability. */
	UPROPERTY(Replicated)
	FGameplayTag CurrentAbilityTag;

	/** Combo counters indexed by tag. */
	UPROPERTY()
	TMap<FGameplayTag, int32> ComboCounters;

	/** Runtime cooldown end time per ability tag (server/local authority clock seconds). */
	TMap<FGameplayTag, float> AbilityCooldownEndTimes;

	/**
	 * Grant-source tracking: skill tags granted by mastery reconciliation.
	 *
	 * ONLY abilities granted through ReconcileMasteryGrantedAbilities() are
	 * recorded here. This is the authoritative set used to determine which
	 * abilities can be safely revoked during future reconciliation passes.
	 *
	 * Rule: if a tag is in this set, the ASC owns the grant and may revoke it.
	 * If a tag is NOT in this set, the ASC does not own the grant and must not revoke it.
	 */
	TSet<FGameplayTag> MasteryGrantedSkills;

	void CleanupExpiredCooldowns() const;
};
