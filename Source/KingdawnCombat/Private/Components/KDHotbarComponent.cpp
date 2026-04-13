// Copyright Kingdawn. All Rights Reserved.

#include "Components/KDHotbarComponent.h"
#include "Components/KDAbilitySystemComponent.h"
#include "Abilities/KDGA_ComboAttack.h"
#include "Characters/KDCombatCharacter.h"
#include "Core/KDGameplayTags.h"
#include "Data/KDBuildDefinition.h"
#include "Data/KDHotbarLayoutDefinition.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Mastery/KDMasteryComponent.h"
#include "Mastery/KDMasteryTestBootstrap.h"

namespace
{
	FGameplayTag ResolveComboLightTag()
	{
		if (FKDGameplayTags::Combat_Skill_Combo_Light.IsValid())
		{
			return FKDGameplayTags::Combat_Skill_Combo_Light;
		}

		return FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Combat.Skill.Combo.Light")), false);
	}

	FGameplayTag ResolveTag(const FGameplayTag& NativeTag, const TCHAR* TagName)
	{
		if (NativeTag.IsValid())
		{
			return NativeTag;
		}

		return FGameplayTag::RequestGameplayTag(FName(TagName), false);
	}

	AKDCombatCharacter* ResolveCombatCharacter(AActor* Owner)
	{
		if (!Owner)
		{
			return nullptr;
		}

		if (APlayerController* PC = Cast<APlayerController>(Owner))
		{
			return Cast<AKDCombatCharacter>(PC->GetPawn());
		}

		return Cast<AKDCombatCharacter>(Owner);
	}

	void AddIfValid(TArray<FGameplayTag>& Tags, const FGameplayTag& Tag)
	{
		if (Tag.IsValid())
		{
			Tags.Add(Tag);
		}
	}

	TArray<FGameplayTag> GetBladeShieldPlaceholderTags()
	{
		TArray<FGameplayTag> Tags;
		AddIfValid(Tags, ResolveComboLightTag());
		AddIfValid(Tags, ResolveTag(FKDGameplayTags::Skill_BladeShield_HeavyStrike, TEXT("KD.Skill.BladeShield.HeavyStrike")));
		AddIfValid(Tags, ResolveTag(FKDGameplayTags::Skill_BladeShield_ShieldBash, TEXT("KD.Skill.BladeShield.ShieldBash")));
		return Tags;
	}
}

UKDHotbarComponent::UKDHotbarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize 4 bars, each with 9 slots
	Bars.SetNum(KD_HOTBAR_MAX_BARS);
	for (int32 i = 0; i < Bars.Num(); ++i)
	{
		Bars[i].Slots.SetNum(KD_HOTBAR_SLOTS_PER_BAR);
	}

	ActiveBarIndex = 0;
	bMasteryEventsBound = false;
}

void UKDHotbarComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache ASC reference
	CachedASC = GetOwnerAbilitySystemComponent();

	if (AKDCombatCharacter* CombatCharacter = ResolveCombatCharacter(GetOwner()))
	{
		if (const UKDBuildDefinition* ActiveBuild = CombatCharacter->GetActiveBuildDefinition())
		{
			if (!ActiveBuild->DefaultHotbarLayout.IsNull())
			{
				if (UKDHotbarLayoutDefinition* Layout = ActiveBuild->DefaultHotbarLayout.LoadSynchronous())
				{
					ClearAllSlots();
					if (SetupSlotsFromLayout(Layout))
					{
						UE_LOG(LogTemp, Log, TEXT("KDHotbar: Applied build-driven hotbar layout '%s' during BeginPlay"),
							*Layout->GetName());
						return;
					}
				}
			}
		}
	}

	// Fallback to branch defaults if no build layout was available yet.
	SetupDefaultSlots();

	// Bind to mastery component events for automatic hotbar reconciliation.
	// This ensures that when skills are unlocked or masteries are activated,
	// the hotbar automatically places or removes the corresponding skills.
	// This binding happens AFTER build layout and default setup to avoid
	// interfering with initialization order.
	if (!bMasteryEventsBound)
	{
		if (UKDMasteryComponent* MasteryComp = GetOwnerMasteryComponent())
		{
			MasteryComp->OnSkillUnlocked.AddDynamic(this, &UKDHotbarComponent::OnMasterySkillUnlocked);
			MasteryComp->OnMasteryActivated.AddDynamic(this, &UKDHotbarComponent::OnMasteryActivated);
			bMasteryEventsBound = true;
			
			UE_LOG(LogTemp, Log, TEXT("KDHotbar: Bound to mastery component events on %s"), *GetNameSafe(GetOwner()));
		}
	}
}

// --- Bar/Page Management ---

void UKDHotbarComponent::SetActiveBar(int32 BarIndex)
{
	if (!Bars.IsValidIndex(BarIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Invalid bar index %d (max %d)"), BarIndex, Bars.Num() - 1);
		return;
	}

	if (ActiveBarIndex != BarIndex)
	{
		const int32 PreviousBar = ActiveBarIndex;
		ActiveBarIndex = BarIndex;

		UE_LOG(LogTemp, Log, TEXT("KDHotbar: Switched from bar %d to bar %d"), PreviousBar, ActiveBarIndex);
		OnActiveBarChanged.Broadcast(ActiveBarIndex);
	}
}

const FKDHotbarBar& UKDHotbarComponent::GetActiveBar() const
{
	return GetBar(ActiveBarIndex);
}

const FKDHotbarBar& UKDHotbarComponent::GetBar(int32 BarIndex) const
{
	if (Bars.IsValidIndex(BarIndex))
	{
		return Bars[BarIndex];
	}
	static FKDHotbarBar EmptyBar;
	return EmptyBar;
}

// --- Slot Management (Active Bar) ---

void UKDHotbarComponent::AssignSlot(int32 SlotIndex, FGameplayTag AbilityTag)
{
	AssignSlotOnBar(ActiveBarIndex, SlotIndex, AbilityTag);
}

void UKDHotbarComponent::ClearSlot(int32 SlotIndex)
{
	ClearSlotOnBar(ActiveBarIndex, SlotIndex);
}

void UKDHotbarComponent::SwapSlots(int32 SlotIndexA, int32 SlotIndexB)
{
	if (!Bars.IsValidIndex(ActiveBarIndex))
	{
		return;
	}

	FKDHotbarBar& ActiveBar = Bars[ActiveBarIndex];
	if (!ActiveBar.Slots.IsValidIndex(SlotIndexA) || !ActiveBar.Slots.IsValidIndex(SlotIndexB))
	{
		return;
	}

	ActiveBar.Slots.Swap(SlotIndexA, SlotIndexB);
}

FGameplayTag UKDHotbarComponent::GetSlotAbilityTag(int32 SlotIndex) const
{
	return GetSlotAbilityTagOnBar(ActiveBarIndex, SlotIndex);
}

bool UKDHotbarComponent::IsSlotAssigned(int32 SlotIndex) const
{
	if (!Bars.IsValidIndex(ActiveBarIndex))
	{
		return false;
	}

	return Bars[ActiveBarIndex].IsSlotAssigned(SlotIndex);
}

// --- Slot Management (Any Bar) ---

void UKDHotbarComponent::AssignSlotOnBar(int32 BarIndex, int32 SlotIndex, FGameplayTag AbilityTag)
{
	if (!Bars.IsValidIndex(BarIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Invalid bar index %d (max %d)"), BarIndex, Bars.Num() - 1);
		return;
	}

	FKDHotbarBar& Bar = Bars[BarIndex];
	if (!Bar.Slots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Invalid slot index %d on bar %d"), SlotIndex, BarIndex);
		return;
	}

	if (!AbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Cannot assign invalid tag to bar %d slot %d"), BarIndex, SlotIndex);
		return;
	}

	Bar.Slots[SlotIndex] = FKDHotbarSlot(AbilityTag);

	// Log the assignment with a note about compatibility
	// The hotbar does NOT gate skills by class/branch — that's the ability system's job.
	// This log helps debugging if a player somehow gets an incompatible skill on their bar.
	UE_LOG(LogTemp, Log, TEXT("KDHotbar: Assigned %s to bar %d slot %d"), *AbilityTag.ToString(), BarIndex, SlotIndex);
}

void UKDHotbarComponent::ClearSlotOnBar(int32 BarIndex, int32 SlotIndex)
{
	if (!Bars.IsValidIndex(BarIndex))
	{
		return;
	}

	FKDHotbarBar& Bar = Bars[BarIndex];
	if (!Bar.Slots.IsValidIndex(SlotIndex))
	{
		return;
	}

	Bar.Slots[SlotIndex] = FKDHotbarSlot();
}

FGameplayTag UKDHotbarComponent::GetSlotAbilityTagOnBar(int32 BarIndex, int32 SlotIndex) const
{
	if (!Bars.IsValidIndex(BarIndex))
	{
		return FGameplayTag();
	}

	return Bars[BarIndex].GetSlotTag(SlotIndex);
}

// --- Activation ---

bool UKDHotbarComponent::ActivateSlot(int32 SlotIndex)
{
	UE_LOG(LogTemp, Log, TEXT("KDHotbar: ActivateSlot called on active bar %d - SlotIndex=%d"), ActiveBarIndex, SlotIndex);

	if (!Bars.IsValidIndex(ActiveBarIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Invalid active bar index %d"), ActiveBarIndex);
		return false;
	}

	FKDHotbarBar& ActiveBar = Bars[ActiveBarIndex];
	if (!ActiveBar.Slots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Invalid slot index %d on active bar %d"), SlotIndex, ActiveBarIndex);
		return false;
	}

	const FKDHotbarSlot& Slot = ActiveBar.Slots[SlotIndex];
	if (!Slot.IsAssigned())
	{
		// Auto-setup defaults on first access
		SetupDefaultSlots();

		// Retry after defaults
		if (!ActiveBar.Slots[SlotIndex].IsAssigned())
		{
			UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Slot %d on bar %d is still not assigned after defaults"), SlotIndex, ActiveBarIndex);
			return false;
		}
	}

	const FGameplayTag AbilityTag = ActiveBar.Slots[SlotIndex].AbilityTag;
	UE_LOG(LogTemp, Log, TEXT("KDHotbar: Active bar %d slot %d resolved to tag=%s"), ActiveBarIndex, SlotIndex, *AbilityTag.ToString());

	if (!CachedASC)
	{
		CachedASC = GetOwnerAbilitySystemComponent();
	}

	if (!CachedASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: No ASC found for activation on bar %d slot %d"), ActiveBarIndex, SlotIndex);
		return false;
	}

	if (!CachedASC->GetAbilityHandleByTag(AbilityTag).IsValid())
	{
		if (AKDCombatCharacter* CombatCharacter = ResolveCombatCharacter(GetOwner()))
		{
			if (const UKDBuildDefinition* ActiveBuild = CombatCharacter->GetActiveBuildDefinition())
			{
				if (!ActiveBuild->DefaultHotbarLayout.IsNull())
				{
					if (UKDHotbarLayoutDefinition* Layout = ActiveBuild->DefaultHotbarLayout.LoadSynchronous())
					{
						ClearAllSlots();
						if (SetupSlotsFromLayout(Layout))
						{
							const FGameplayTag RebuiltTag = GetSlotAbilityTagOnBar(ActiveBarIndex, SlotIndex);
							if (RebuiltTag.IsValid() && RebuiltTag != AbilityTag)
							{
								UE_LOG(LogTemp, Log, TEXT("KDHotbar: Rebuilt stale hotbar state from build layout, slot %d now resolves to %s"),
									SlotIndex, *RebuiltTag.ToString());
								return ActivateSlot(SlotIndex);
							}
						}
					}
				}
			}
		}
	}

	// --- Smart combo routing ---
	// If the active ability matches this slot's tag AND it's a combo ability,
	// send combo continuation input instead of re-triggering the ability.
	if (IsActiveComboAbility(AbilityTag))
	{
		if (TrySendComboInput())
		{
			UE_LOG(LogTemp, Log, TEXT("KDHotbar: Combo continuation sent for %s (bar %d slot %d)"), *AbilityTag.ToString(), ActiveBarIndex, SlotIndex);
			return true;
		}
	}

	// Skills should route through the character so range acquisition and basic-attack
	// suppression stay in one place.
	AActor* OwnerActor = GetOwner();
	AKDCombatCharacter* CombatCharacter = nullptr;
	if (APlayerController* PC = Cast<APlayerController>(OwnerActor))
	{
		CombatCharacter = Cast<AKDCombatCharacter>(PC->GetPawn());
	}
	else
	{
		CombatCharacter = Cast<AKDCombatCharacter>(OwnerActor);
	}

	bool bTriggered = false;
	if (CombatCharacter)
	{
		UE_LOG(LogTemp, Log, TEXT("KDHotbar: Routing bar %d slot %d through character activation for %s"), ActiveBarIndex, SlotIndex, *AbilityTag.ToString());
		bTriggered = CombatCharacter->ActivateAbilityByTagWithAutoApproach(AbilityTag);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("KDHotbar: Attempting TriggerAbilityByTag(%s) for bar %d slot %d"), *AbilityTag.ToString(), ActiveBarIndex, SlotIndex);
		bTriggered = CachedASC->TriggerAbilityByTag(AbilityTag);
	}

	if (bTriggered)
	{
		UE_LOG(LogTemp, Log, TEXT("KDHotbar: SUCCESS - Activated bar %d slot %d (%s)"), ActiveBarIndex, SlotIndex, *AbilityTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("KDHotbar: FAILED to activate bar %d slot %d (%s)"), ActiveBarIndex, SlotIndex, *AbilityTag.ToString());
	}

	return bTriggered;
}

// --- Initialization ---

void UKDHotbarComponent::SetupDefaultSlots()
{
	// Branch-aware Silkroad-style defaults.
	// Bar 0 is populated from the current combat branch.
	// Basic attack remains on mouse double-click and is never placed on the hotbar.

	AKDCombatCharacter* CombatCharacter = Cast<AKDCombatCharacter>(GetOwner());
	if (!CombatCharacter)
	{
		if (const APlayerController* PC = Cast<APlayerController>(GetOwner()))
		{
			CombatCharacter = Cast<AKDCombatCharacter>(PC->GetPawn());
		}
	}

	const FKDMasteryIdentity Identity = CombatCharacter ? CombatCharacter->GetMasteryIdentity() : FKDMasteryIdentity(EKDMasteryClass::Warrior, EKDMasteryBranch::BladeShield);

	// Authoritative reset: always clear bar 0 to a known-clean state before
	// applying any defaults. This prevents stale state from previous
	// SetupDefaultSlots / reconcile cycles from persisting.
	for (int32 SlotIndex = 0; SlotIndex < KD_HOTBAR_SLOTS_PER_BAR; ++SlotIndex)
	{
		ClearSlotOnBar(0, SlotIndex);
	}

	// Mastery test mode: clear tracked mastery placements, then rebuild
	// purely from mastery-unlocked skills. This is the authoritative path
	// that guarantees the hotbar reflects actual mastery state.
	const bool bHasBuildDefinition = CombatCharacter && CombatCharacter->GetActiveBuildDefinition();
	if (!bHasBuildDefinition && FKDMasteryTestBootstrap::IsTestModeEnabled())
	{
		// Reset mastery tracking so reconcile treats this as a fresh placement.
		MasteryPlacedSkills.Empty();

		const int32 ReconciledCount = ReconcileMasteryHotbar();
		UE_LOG(LogTemp, Log, TEXT("KDHotbar: Test mode authoritative rebuild - mastery reconciliation placed %d skills"), ReconciledCount);

		// Log final slot state for verification.
		if (Bars.IsValidIndex(0))
		{
			const FKDHotbarBar& Bar0 = Bars[0];
			for (int32 SlotIdx = 0; SlotIdx < Bar0.Slots.Num(); ++SlotIdx)
			{
				if (Bar0.Slots[SlotIdx].IsAssigned())
				{
					UE_LOG(LogTemp, Log, TEXT("KDHotbar: Bar 0 Slot %d = %s"), SlotIdx, *Bar0.Slots[SlotIdx].AbilityTag.ToString());
				}
			}
		}
		return;
	}

	if (Identity.MasteryBranch == EKDMasteryBranch::Wizard)
	{
		const FGameplayTag ArcaneBoltTag = ResolveTag(FKDGameplayTags::Skill_Wizard_ArcaneBolt, TEXT("KD.Skill.Wizard.ArcaneBolt"));
		const FGameplayTag FireballTag = ResolveTag(FKDGameplayTags::Skill_Wizard_Fireball, TEXT("KD.Skill.Wizard.Fireball"));
		const FGameplayTag FrostNovaTag = ResolveTag(FKDGameplayTags::Skill_Wizard_FrostNova, TEXT("KD.Skill.Wizard.FrostNova"));
		const FGameplayTag ManaShieldTag = ResolveTag(FKDGameplayTags::Skill_Wizard_ManaShield, TEXT("KD.Skill.Wizard.ManaShield"));

		if (ArcaneBoltTag.IsValid()) { AssignSlotOnBar(0, 0, ArcaneBoltTag); } else { UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Arcane Bolt tag is invalid during SetupDefaultSlots")); }
		if (FireballTag.IsValid()) { AssignSlotOnBar(0, 1, FireballTag); } else { UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Fireball tag is invalid during SetupDefaultSlots")); }
		if (FrostNovaTag.IsValid()) { AssignSlotOnBar(0, 2, FrostNovaTag); } else { UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Frost Nova tag is invalid during SetupDefaultSlots")); }
		if (ManaShieldTag.IsValid()) { AssignSlotOnBar(0, 3, ManaShieldTag); } else { UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Mana Shield tag is invalid during SetupDefaultSlots")); }

		UE_LOG(LogTemp, Log, TEXT("KDHotbar: Default slots configured for Wizard (Bar0: 1=ArcaneBolt, 2=Fireball, 3=FrostNova, 4=ManaShield)"));
		return;
	}

	const FGameplayTag ComboTag = ResolveComboLightTag();
	const FGameplayTag HeavyStrikeTag = ResolveTag(FKDGameplayTags::Skill_BladeShield_HeavyStrike, TEXT("KD.Skill.BladeShield.HeavyStrike"));
	const FGameplayTag ShieldBashTag = ResolveTag(FKDGameplayTags::Skill_BladeShield_ShieldBash, TEXT("KD.Skill.BladeShield.ShieldBash"));

	if (ComboTag.IsValid())
	{
		AssignSlotOnBar(0, 0, ComboTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Combo tag is invalid during SetupDefaultSlots"));
	}

	if (HeavyStrikeTag.IsValid())
	{
		AssignSlotOnBar(0, 1, HeavyStrikeTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Heavy strike tag is invalid during SetupDefaultSlots"));
	}

	if (ShieldBashTag.IsValid())
	{
		AssignSlotOnBar(0, 2, ShieldBashTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar: Shield bash tag is invalid during SetupDefaultSlots"));
	}

	UE_LOG(LogTemp, Log, TEXT("KDHotbar: Default slots configured for BladeShield (Bar0: 1=Combo, 2=HeavyStrike, 3=ShieldBash)"));
}

void UKDHotbarComponent::ClearAllSlots()
{
	for (int32 BarIdx = 0; BarIdx < Bars.Num(); ++BarIdx)
	{
		FKDHotbarBar& Bar = Bars[BarIdx];
		for (int32 SlotIdx = 0; SlotIdx < Bar.Slots.Num(); ++SlotIdx)
		{
			Bar.Slots[SlotIdx] = FKDHotbarSlot();
		}
	}

	ActiveBarIndex = 0;
	UE_LOG(LogTemp, Log, TEXT("KDHotbar: Cleared all slots on all bars"));
}

bool UKDHotbarComponent::SetupSlotsFromLayout(UKDHotbarLayoutDefinition* Layout)
{
	if (!Layout)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar::SetupSlotsFromLayout - null layout"));
		return false;
	}

	const int32 TargetBar = Layout->BarIndex;
	if (!Bars.IsValidIndex(TargetBar))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar::SetupSlotsFromLayout - bar index %d out of range (max %d)"),
			TargetBar, Bars.Num() - 1);
		return false;
	}

	// Clear the target bar first
	FKDHotbarBar& Bar = Bars[TargetBar];
	for (int32 SlotIdx = 0; SlotIdx < Bar.Slots.Num(); ++SlotIdx)
	{
		Bar.Slots[SlotIdx] = FKDHotbarSlot();
	}

	// Assign slots from layout
	int32 AssignedCount = 0;
	for (int32 SlotIdx = 0; SlotIdx < Layout->Slots.Num() && SlotIdx < KD_HOTBAR_SLOTS_PER_BAR; ++SlotIdx)
	{
		const FGameplayTag& Tag = Layout->Slots[SlotIdx].AbilityTag;
		if (Tag.IsValid())
		{
			AssignSlotOnBar(TargetBar, SlotIdx, Tag);
			++AssignedCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("KDHotbar::SetupSlotsFromLayout - applied layout '%s' to bar %d (%d slots assigned)"),
		*Layout->GetName(), TargetBar, AssignedCount);
	return AssignedCount > 0;
}

// --- Helpers ---

UKDAbilitySystemComponent* UKDHotbarComponent::GetOwnerAbilitySystemComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// First try the owner itself.
	if (UKDAbilitySystemComponent* ASC = Owner->FindComponentByClass<UKDAbilitySystemComponent>())
	{
		return ASC;
	}

	// Hotbar is currently owned by KDPlayerController, so the runtime ASC normally
	// lives on the possessed pawn, not on the controller itself.
	if (const APlayerController* PC = Cast<APlayerController>(Owner))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			return Pawn->FindComponentByClass<UKDAbilitySystemComponent>();
		}
	}

	return nullptr;
}

bool UKDHotbarComponent::IsActiveComboAbility(const FGameplayTag& AbilityTag) const
{
	if (!CachedASC)
	{
		return false;
	}

	// Check if the currently active ability matches this slot's tag
	FGameplayTag CurrentTag = CachedASC->GetCurrentAbilityTag();
	if (CurrentTag != AbilityTag)
	{
		return false;
	}

	// Check if the active ability is a combo ability
	UGameplayAbility* ActiveAbility = CachedASC->GetAbilityByTag(CurrentTag);
	if (!ActiveAbility)
	{
		return false;
	}

	return Cast<UKDGA_ComboAttack>(ActiveAbility) != nullptr;
}

bool UKDHotbarComponent::TrySendComboInput()
{
	if (!CachedASC)
	{
		return false;
	}

	FGameplayTag CurrentTag = CachedASC->GetCurrentAbilityTag();
	if (!CurrentTag.IsValid())
	{
		return false;
	}

	UGameplayAbility* ActiveAbility = CachedASC->GetAbilityByTag(CurrentTag);
	if (!ActiveAbility)
	{
		return false;
	}

	UKDGA_ComboAttack* ComboAbility = Cast<UKDGA_ComboAttack>(ActiveAbility);
	if (!ComboAbility)
	{
		return false;
	}

	// Send combo input to the active combo ability
	ComboAbility->SendComboInput();
	return true;
}

// ========================================================================
// Mastery Hotbar Reconciliation
// ========================================================================

UKDMasteryComponent* UKDHotbarComponent::GetOwnerMasteryComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Same resolution pattern as GetOwnerAbilitySystemComponent:
	// hotbar is owned by KDPlayerController, mastery component is on the pawn.
	if (UKDMasteryComponent* MasteryComp = Owner->FindComponentByClass<UKDMasteryComponent>())
	{
		return MasteryComp;
	}

	if (const APlayerController* PC = Cast<APlayerController>(Owner))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			return Pawn->FindComponentByClass<UKDMasteryComponent>();
		}
	}

	return nullptr;
}

int32 UKDHotbarComponent::ReconcileMasteryHotbar()
{
	UKDMasteryComponent* MasteryComp = GetOwnerMasteryComponent();
	if (!MasteryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar::ReconcileMastery: No mastery component found. Skipping."));
		return 0;
	}

	int32 ChangesMade = 0;

	// 1. Collect mastery-unlocked skill tags
	const TArray<FGameplayTag> UnlockedSkills = MasteryComp->GetAllUnlockedSkills();
	const TSet<FGameplayTag> UnlockedSet(UnlockedSkills);

	UE_LOG(LogTemp, Log, TEXT("KDHotbar::ReconcileMastery: %d mastery-unlocked skills, %d mastery-placed on bar."),
		UnlockedSkills.Num(), MasteryPlacedSkills.Num());

	// 2. Remove stale mastery-placed skills (tracked as mastery-placed but no longer unlocked).
	//    ONLY skills in MasteryPlacedSkills are candidates for removal.
	TArray<FGameplayTag> TagsToRemove;
	for (const FGameplayTag& PlacedTag : MasteryPlacedSkills)
	{
		if (!UnlockedSet.Contains(PlacedTag))
		{
			TagsToRemove.Add(PlacedTag);
		}
	}

	for (const FGameplayTag& StaleTag : TagsToRemove)
	{
		// Find and clear the slot containing this tag on any bar.
		for (int32 BarIdx = 0; BarIdx < Bars.Num(); ++BarIdx)
		{
			FKDHotbarBar& Bar = Bars[BarIdx];
			for (int32 SlotIdx = 0; SlotIdx < Bar.Slots.Num(); ++SlotIdx)
			{
				if (Bar.Slots[SlotIdx].Matches(StaleTag))
				{
					Bar.Slots[SlotIdx] = FKDHotbarSlot();
					UE_LOG(LogTemp, Log, TEXT("KDHotbar::ReconcileMastery: Removed stale mastery skill %s from bar %d slot %d."),
						*StaleTag.ToString(), BarIdx, SlotIdx);
					break;
				}
			}
		}

		MasteryPlacedSkills.Remove(StaleTag);
		++ChangesMade;
	}

	// In mastery test mode, scrub BladeShield placeholder slots that were injected by
	// legacy defaults but were never actually unlocked from mastery.
	if (FKDMasteryTestBootstrap::IsTestModeEnabled())
	{
		if (const AKDCombatCharacter* CombatCharacter = ResolveCombatCharacter(GetOwner()))
		{
			if (CombatCharacter->GetMasteryIdentity().MasteryBranch == EKDMasteryBranch::BladeShield)
			{
				for (const FGameplayTag& PlaceholderTag : GetBladeShieldPlaceholderTags())
				{
					if (UnlockedSet.Contains(PlaceholderTag))
					{
						continue;
					}

					for (int32 BarIdx = 0; BarIdx < Bars.Num(); ++BarIdx)
					{
						FKDHotbarBar& Bar = Bars[BarIdx];
						for (int32 SlotIdx = 0; SlotIdx < Bar.Slots.Num(); ++SlotIdx)
						{
							if (Bar.Slots[SlotIdx].Matches(PlaceholderTag))
							{
								Bar.Slots[SlotIdx] = FKDHotbarSlot();
								MasteryPlacedSkills.Remove(PlaceholderTag);
								++ChangesMade;
								UE_LOG(LogTemp, Log, TEXT("KDHotbar::ReconcileMastery: Removed legacy placeholder %s from bar %d slot %d."),
									*PlaceholderTag.ToString(), BarIdx, SlotIdx);
							}
						}
					}
				}
			}
		}
	}

	// 2b. Compact mastery-placed skills forward to fill gaps left by removed placeholders.
	//     Example: after Combo.Light is removed from slot 0, HeavyStrike (slot 1) and
	//     ShieldBash (slot 2) should shift to slots 0 and 1 so keys 1-2 map correctly.
	if (Bars.IsValidIndex(0) && ChangesMade > 0)
	{
		FKDHotbarBar& Bar0 = Bars[0];
		int32 WriteIdx = 0;
		for (int32 ReadIdx = 0; ReadIdx < Bar0.Slots.Num(); ++ReadIdx)
		{
			if (Bar0.Slots[ReadIdx].IsAssigned())
			{
				if (WriteIdx != ReadIdx)
				{
					UE_LOG(LogTemp, Log, TEXT("KDHotbar::ReconcileMastery: Compacting slot %d -> %d (%s)"),
						ReadIdx, WriteIdx, *Bar0.Slots[ReadIdx].AbilityTag.ToString());
					Bar0.Slots[WriteIdx] = Bar0.Slots[ReadIdx];
					Bar0.Slots[ReadIdx] = FKDHotbarSlot();
				}
				++WriteIdx;
			}
		}
	}

	// 3. Place unlocked mastery skills that are not yet on any bar.
	//    Only place onto bar 0 (primary bar) in the first available empty slot.
	//    Do NOT claim slots that already have skills assigned (manual or build).
	if (!Bars.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDHotbar::ReconcileMastery: Bar 0 does not exist. Cannot place skills."));
		return ChangesMade;
	}

	// Build a set of tags already on any bar to avoid duplicates.
	TSet<FGameplayTag> AlreadyOnBar;
	for (const FKDHotbarBar& Bar : Bars)
	{
		for (const FKDHotbarSlot& Slot : Bar.Slots)
		{
			if (Slot.IsAssigned())
			{
				AlreadyOnBar.Add(Slot.AbilityTag);
			}
		}
	}

	FKDHotbarBar& PrimaryBar = Bars[0];
	for (const FGameplayTag& SkillTag : UnlockedSkills)
	{
		// Skip if already placed somewhere on the bar.
		if (AlreadyOnBar.Contains(SkillTag))
		{
			continue;
		}

		// Find the first empty slot on bar 0.
		int32 EmptySlot = INDEX_NONE;
		for (int32 SlotIdx = 0; SlotIdx < PrimaryBar.Slots.Num(); ++SlotIdx)
		{
			if (!PrimaryBar.Slots[SlotIdx].IsAssigned())
			{
				EmptySlot = SlotIdx;
				break;
			}
		}

		if (EmptySlot == INDEX_NONE)
		{
			UE_LOG(LogTemp, Log, TEXT("KDHotbar::ReconcileMastery: No empty slots on bar 0. Skill %s not placed."),
				*SkillTag.ToString());
			continue;
		}

		PrimaryBar.Slots[EmptySlot] = FKDHotbarSlot(SkillTag);
		MasteryPlacedSkills.Add(SkillTag);
		AlreadyOnBar.Add(SkillTag);
		++ChangesMade;

		UE_LOG(LogTemp, Log, TEXT("KDHotbar::ReconcileMastery: Placed mastery skill %s on bar 0 slot %d."),
			*SkillTag.ToString(), EmptySlot);
	}

	UE_LOG(LogTemp, Log, TEXT("KDHotbar::ReconcileMastery: Complete. Changes=%d, Tracked=%d."),
		ChangesMade, MasteryPlacedSkills.Num());

	return ChangesMade;
}

bool UKDHotbarComponent::IsMasteryPlacedSkill(FGameplayTag SkillTag) const
{
	return MasteryPlacedSkills.Contains(SkillTag);
}

// ========================================================================
// Mastery Event Handlers
// ========================================================================

void UKDHotbarComponent::OnMasterySkillUnlocked(FGameplayTag SkillTag, FKDMasteryIdentity MasteryIdentity)
{
	UE_LOG(LogTemp, Log, TEXT("KDHotbar: OnMasterySkillUnlocked(%s) — triggering hotbar reconciliation"), *SkillTag.ToString());
	ReconcileMasteryHotbar();
}

void UKDHotbarComponent::OnMasteryActivated(FKDMasteryIdentity MasteryIdentity)
{
	const FString IdentityText = MasteryIdentity.IsValid()
		? FString::Printf(TEXT("Class=%d Branch=%d"),
			static_cast<int32>(MasteryIdentity.MasteryClass),
			static_cast<int32>(MasteryIdentity.MasteryBranch))
		: TEXT("Invalid");
	UE_LOG(LogTemp, Log, TEXT("KDHotbar: OnMasteryActivated(%s) - triggering hotbar reconciliation"), *IdentityText);
	ReconcileMasteryHotbar();
}
