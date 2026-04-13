// Copyright Kingdawn. All Rights Reserved.
// Silkroad-style multi-bar hotbar component for ability slot management.
// Supports 4 bars (F1-F4) × 9 slots (1-9 keys).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "KDHotbarComponent.generated.h"

class UKDAbilitySystemComponent;
class UKDHotbarLayoutDefinition;
class UKDMasteryComponent;
class UKDGA_ComboAttack;

/** Maximum number of slots per bar (keys 1-9). */
static constexpr int32 KD_HOTBAR_SLOTS_PER_BAR = 9;

/** Maximum number of hotbar pages/bars (F1-F4). */
static constexpr int32 KD_HOTBAR_MAX_BARS = 4;

/**
 * Hotbar slot data structure.
 * Holds an ability tag that gets activated when the slot is triggered.
 * Slot index never defines skill identity — skill identity comes from the tag.
 */
USTRUCT(BlueprintType)
struct FKDHotbarSlot
{
	GENERATED_BODY()

	/** The ability tag assigned to this slot. Empty = unassigned slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	FGameplayTag AbilityTag;

	FKDHotbarSlot()
		: AbilityTag(FGameplayTag())
	{
	}

	explicit FKDHotbarSlot(FGameplayTag InTag)
		: AbilityTag(InTag)
	{
	}

	/** Check if the slot has an ability assigned. */
	bool IsAssigned() const { return AbilityTag.IsValid(); }

	/** Check if the slot matches a specific ability tag. */
	bool Matches(const FGameplayTag& Tag) const { return AbilityTag == Tag; }
};

/**
 * A single hotbar bar/page containing up to 9 slots.
 */
USTRUCT(BlueprintType)
struct FKDHotbarBar
{
	GENERATED_BODY()

	/** Slots on this bar (index 0-8 = keys 1-9). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	TArray<FKDHotbarSlot> Slots;

	FKDHotbarBar()
	{
		Slots.SetNum(KD_HOTBAR_SLOTS_PER_BAR);
	}

	/** Get the ability tag at a slot index. */
	FGameplayTag GetSlotTag(int32 SlotIndex) const
	{
		if (Slots.IsValidIndex(SlotIndex))
		{
			return Slots[SlotIndex].AbilityTag;
		}
		return FGameplayTag();
	}

	/** Check if a slot is assigned. */
	bool IsSlotAssigned(int32 SlotIndex) const
	{
		return Slots.IsValidIndex(SlotIndex) && Slots[SlotIndex].IsAssigned();
	}
};

/**
 * Delegate broadcast when the active hotbar page changes (F1-F4).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKDOnActiveBarChanged, int32, NewBarIndex);

/**
 * Silkroad-style multi-bar hotbar component.
 * 
 * Architecture:
 * - 4 bars/pages (F1-F4 to switch)
 * - 9 slots per bar (keys 1-9)
 * - Active bar index determines which bar receives key input
 * - Slot index never defines skill identity
 * - Player can arrange skills freely across bars
 * - Skill identity is always the gameplay tag, never the slot position
 * 
 * Smart combo handling:
 * - If the active ability is a combo and the slot's ability matches, routes to SendComboInput()
 * - Otherwise, triggers the ability normally
 * 
 * Branch/weapon compatibility:
 * - Skill activation checks are deferred to the ability system
 * - Hotbar does NOT gate skills by class/branch (that's the ability system's job)
 * - Hotbar simply maps key positions to ability tags
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDHotbarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDHotbarComponent();

	// --- Bar/Page Management ---

	/** Get the current active bar index (0-3). */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	int32 GetActiveBarIndex() const { return ActiveBarIndex; }

	/** Get the total number of bars. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	int32 GetBarCount() const { return Bars.Num(); }

	/**
	 * Switch to a specific hotbar page.
	 * @param BarIndex The bar index (0-3 for F1-F4).
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void SetActiveBar(int32 BarIndex);

	/** Get the active bar's slot data. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	const FKDHotbarBar& GetActiveBar() const;

	/** Get a specific bar's slot data. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	const FKDHotbarBar& GetBar(int32 BarIndex) const;

	// --- Slot Management (Active Bar) ---

	/**
	 * Assign an ability to a slot on the active bar.
	 * @param SlotIndex The slot index (0-8).
	 * @param AbilityTag The ability tag to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void AssignSlot(int32 SlotIndex, FGameplayTag AbilityTag);

	/**
	 * Clear a slot on the active bar.
	 * @param SlotIndex The slot index to clear.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void ClearSlot(int32 SlotIndex);

	/**
	 * Swap two slots on the active bar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void SwapSlots(int32 SlotIndexA, int32 SlotIndexB);

	/**
	 * Get the ability tag at a slot on the active bar.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	FGameplayTag GetSlotAbilityTag(int32 SlotIndex) const;

	/**
	 * Check if a slot on the active bar is assigned.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	bool IsSlotAssigned(int32 SlotIndex) const;

	// --- Slot Management (Any Bar) ---

	/**
	 * Assign an ability to a slot on a specific bar.
	 * @param BarIndex The bar index (0-3).
	 * @param SlotIndex The slot index (0-8).
	 * @param AbilityTag The ability tag to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void AssignSlotOnBar(int32 BarIndex, int32 SlotIndex, FGameplayTag AbilityTag);

	/**
	 * Clear a slot on a specific bar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void ClearSlotOnBar(int32 BarIndex, int32 SlotIndex);

	/**
	 * Get the ability tag at a slot on a specific bar.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	FGameplayTag GetSlotAbilityTagOnBar(int32 BarIndex, int32 SlotIndex) const;

	// --- Activation ---

	/**
	 * Activate the ability in the given slot on the active bar.
	 * 
	 * Flow:
	 * 1. Resolve active bar
	 * 2. Check if slot is valid and assigned
	 * 3. Get the ability tag from the slot
	 * 4. Check if the active ability is a combo and the slot matches
	 * 5a. If combo match: call SendComboInput() to continue combo
	 * 5b. Otherwise: call TriggerAbilityByTag() normally
	 * 
	 * @param SlotIndex The slot index to activate (0-8 on active bar).
	 * @return True if activation was attempted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	bool ActivateSlot(int32 SlotIndex);

	// --- Initialization ---

	/**
	 * Set up default slot assignments for all bars.
	 * Called during BeginPlay if no custom assignment is made.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void SetupDefaultSlots();

	/**
	 * Clear all slots on all bars (reset to empty state).
	 * Used to force a clean state before re-applying defaults.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	void ClearAllSlots();

	/**
	 * Setup slots from a build-driven hotbar layout definition.
	 * Clears the target bar first, then assigns slots from the layout.
	 *
	 * @param Layout The hotbar layout definition to apply (must be valid)
	 * @return True if layout was applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Hotbar")
	bool SetupSlotsFromLayout(UKDHotbarLayoutDefinition* Layout);

	// --- Delegates ---

	/** Broadcast when the active bar changes (F1-F4). */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn|Hotbar")
	FKDOnActiveBarChanged OnActiveBarChanged;

	// --- Query Helpers (for UI) ---

	/** Get the number of slots per bar. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	int32 GetSlotCount() const { return KD_HOTBAR_SLOTS_PER_BAR; }

	/** Get all bars (for UI). */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Hotbar")
	const TArray<FKDHotbarBar>& GetAllBars() const { return Bars; }

	// ========================================================================
	// Mastery Hotbar Reconciliation
	// ========================================================================

	/**
	 * Reconcile hotbar from mastery unlock state.
	 *
	 * - Places unlocked mastery skills onto empty slots on bar 0 (first available).
	 * - Removes slots that were mastery-placed but are no longer unlocked.
	 * - Never touches manually-assigned or build-assigned slots (only tracks mastery-placed).
	 *
	 * TRACKING RULE:
	 * MasteryPlacedSkills is a TSet<FGameplayTag> that records which skills
	 * were placed BY THIS RECONCILIATION PATH. Only skills in this set can
	 * be removed by future reconciliation. Manual slot assignments and build
	 * layout assignments are never tracked and never removed.
	 *
	 * INTENDED CALL SITES:
	 * - After mastery state is loaded/restored
	 * - After a skill is unlocked (OnSkillUnlocked listener)
	 * - Manual call from test harness or Blueprint
	 *
	 * @return Number of slots placed or removed during this reconciliation pass.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Mastery")
	int32 ReconcileMasteryHotbar();

	/**
	 * Check if a skill tag was placed by mastery reconciliation.
	 * Returns false for manually-assigned or build-assigned slots.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Mastery")
	bool IsMasteryPlacedSkill(FGameplayTag SkillTag) const;

protected:
	virtual void BeginPlay() override;

	// ========================================================================
	// Mastery Event Handlers (called by KDMasteryComponent delegates)
	// ========================================================================

	/** Called when a skill is unlocked in the mastery system. Triggers hotbar reconciliation. */
	UFUNCTION()
	void OnMasterySkillUnlocked(FGameplayTag SkillTag, struct FKDMasteryIdentity MasteryIdentity);

	/** Called when a mastery branch is activated. Triggers hotbar reconciliation. */
	UFUNCTION()
	void OnMasteryActivated(struct FKDMasteryIdentity MasteryIdentity);

	/** Get the owning character's ability system component. */
	UKDAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;

	/** Get the mastery component on the owner actor, if present. */
	UKDMasteryComponent* GetOwnerMasteryComponent() const;

	/** Check if the given ability tag is an active combo ability. */
	bool IsActiveComboAbility(const FGameplayTag& AbilityTag) const;

	/** Attempt to send combo input to the active combo ability. */
	bool TrySendComboInput();

private:
	/** Flag to prevent duplicate mastery event binding in BeginPlay. */
	bool bMasteryEventsBound;
	/** All hotbar bars (4 bars, 9 slots each). */
	UPROPERTY(EditAnywhere, Category = "Kingdawn|Hotbar")
	TArray<FKDHotbarBar> Bars;

	/** Currently active bar index (0-3). */
	UPROPERTY(EditAnywhere, Category = "Kingdawn|Hotbar")
	int32 ActiveBarIndex;

	/** Cached reference to the ability system component. */
	UPROPERTY()
	TObjectPtr<UKDAbilitySystemComponent> CachedASC;

	/**
	 * Grant-source tracking: skill tags placed by mastery reconciliation.
	 *
	 * ONLY skills placed through ReconcileMasteryHotbar() are recorded here.
	 * This is the authoritative set used to determine which slots can be
	 * safely removed during future reconciliation passes.
	 *
	 * Rule: if a tag is in this set, the hotbar owns the placement and may remove it.
	 * If a tag is NOT in this set, the hotbar does not own the placement and must not remove it.
	 */
	TSet<FGameplayTag> MasteryPlacedSkills;
};
