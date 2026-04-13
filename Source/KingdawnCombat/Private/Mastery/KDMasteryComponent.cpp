// Copyright Kingdawn. All Rights Reserved.

#include "Mastery/KDMasteryComponent.h"
#include "Mastery/KDMasteryDefinition.h"
#include "Mastery/KDSkillDefinition.h"

UKDMasteryComponent::UKDMasteryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ========================================================================
// Query API
// ========================================================================

bool UKDMasteryComponent::IsSkillUnlocked(FGameplayTag SkillTag) const
{
	if (!SkillTag.IsValid())
	{
		return false;
	}

	// Check across all active masteries
	for (const auto& Pair : UnlockedSkillsMap)
	{
		for (const FGameplayTag& UnlockedTag : Pair.Value)
		{
			if (UnlockedTag == SkillTag)
			{
				return true;
			}
		}
	}

	return false;
}

TArray<FGameplayTag> UKDMasteryComponent::GetUnlockedSkills(FKDMasteryIdentity Identity) const
{
	const TArray<FGameplayTag>* Skills = UnlockedSkillsMap.Find(Identity);
	if (Skills)
	{
		return *Skills;
	}
	return TArray<FGameplayTag>();
}

TArray<FGameplayTag> UKDMasteryComponent::GetAllUnlockedSkills() const
{
	TArray<FGameplayTag> AllSkills;
	for (const auto& Pair : UnlockedSkillsMap)
	{
		AllSkills.Append(Pair.Value);
	}
	return AllSkills;
}

int32 UKDMasteryComponent::GetSpentPoints(FKDMasteryIdentity Identity) const
{
	const int32* Points = SpentPointsMap.Find(Identity);
	return Points ? *Points : 0;
}

UKDSkillDefinition* UKDMasteryComponent::FindSkillDefinition(FGameplayTag SkillTag) const
{
	if (!SkillTag.IsValid())
	{
		return nullptr;
	}

	// Search all loaded mastery definitions for a slot whose skill matches this tag.
	for (const auto& Pair : MasteryDefinitions)
	{
		if (!Pair.Value)
		{
			continue;
		}

		for (const FKDMasterySlot& Slot : Pair.Value->Slots)
		{
			if (Slot.Skill.IsNull())
			{
				continue;
			}

			if (const UKDSkillDefinition* SkillDef = Slot.Skill.LoadSynchronous())
			{
				if (SkillDef->SkillTag == SkillTag)
				{
					return const_cast<UKDSkillDefinition*>(SkillDef);
				}
			}
		}
	}

	return nullptr;
}

bool UKDMasteryComponent::IsMasteryActive(FKDMasteryIdentity Identity) const
{
	return ActiveMasteries.Contains(Identity);
}

bool UKDMasteryComponent::CanUnlockSkill(FGameplayTag SkillTag, FKDMasteryIdentity Identity) const
{
	// 1. Mastery must be active
	if (!IsMasteryActive(Identity))
	{
		return false;
	}

	// 2. Skill must not already be unlocked
	if (IsSkillUnlocked(SkillTag))
	{
		return false;
	}

	// 3. Must have a mastery definition loaded
	const TObjectPtr<UKDMasteryDefinition>* DefPtr = MasteryDefinitions.Find(Identity);
	if (!DefPtr || !*DefPtr)
	{
		return false;
	}

	UKDMasteryDefinition* Def = *DefPtr;

	// 4. Find the slot containing this skill
	const FKDMasterySlot* TargetSlot = nullptr;
	for (const FKDMasterySlot& Slot : Def->Slots)
	{
		if (!Slot.Skill.IsNull())
		{
			// Load the skill definition synchronously for the check.
			// This is acceptable because CanUnlockSkill is a query that
			// designers call from UI; the skill assets should be loaded
			// by then anyway via the mastery definition.
			if (const UKDSkillDefinition* SkillDef = Slot.Skill.LoadSynchronous())
			{
				if (SkillDef->SkillTag == SkillTag)
				{
					TargetSlot = &Slot;
					break;
				}
			}
		}
	}

	if (!TargetSlot)
	{
		// Skill not found in this mastery definition
		return false;
	}

	// 5. Check tier requirement
	const int32 Spent = GetSpentPoints(Identity);
	if (const UKDSkillDefinition* SkillDef = TargetSlot->Skill.Get())
	{
		if (!Def->IsTierUnlocked(SkillDef->RequiredTier, Spent))
		{
			return false;
		}
	}

	// 6. Check mastery point cost
	{
		const int32 Available = GetAvailablePoints(Identity);
		if (const UKDSkillDefinition* SkillDef = TargetSlot->Skill.Get())
		{
			if (Available < SkillDef->MasteryPointCost)
			{
				return false;
			}
		}
	}

	// 7. Check prerequisites (all prerequisite slots must be unlocked)
	const TArray<FGameplayTag>& Unlocked = GetUnlockedSkills(Identity);
	for (const FGameplayTag& PrereqSlotTag : TargetSlot->PrerequisiteSlots)
	{
		// Find the prerequisite slot to get its skill tag
		const FKDMasterySlot* PrereqSlot = Def->FindSlotByTag(PrereqSlotTag);
		if (!PrereqSlot)
		{
			// Invalid prerequisite reference: cannot unlock
			return false;
		}

		if (const UKDSkillDefinition* PrereqSkill = PrereqSlot->Skill.Get())
		{
			bool bPrereqMet = false;
			for (const FGameplayTag& UnlockedTag : Unlocked)
			{
				if (UnlockedTag == PrereqSkill->SkillTag)
				{
					bPrereqMet = true;
					break;
				}
			}
			if (!bPrereqMet)
			{
				return false;
			}
		}
	}

	return true;
}

// ========================================================================
// Mutation API
// ========================================================================

void UKDMasteryComponent::ActivateMastery(FKDMasteryIdentity Identity, UKDMasteryDefinition* Definition)
{
	if (!Identity.IsValid() || !Definition)
	{
		return;
	}

	// Already active: skip
	if (IsMasteryActive(Identity))
	{
		return;
	}

	ActiveMasteries.Add(Identity);
	MasteryDefinitions.Add(Identity, Definition);

	// Initialize state maps if not present
	if (!UnlockedSkillsMap.Contains(Identity))
	{
		UnlockedSkillsMap.Add(Identity, TArray<FGameplayTag>());
	}
	if (!SpentPointsMap.Contains(Identity))
	{
		SpentPointsMap.Add(Identity, 0);
	}
	if (!TotalPointsMap.Contains(Identity))
	{
		TotalPointsMap.Add(Identity, 0);
	}

	OnMasteryActivated.Broadcast(Identity);
}

void UKDMasteryComponent::DeactivateMastery(FKDMasteryIdentity Identity)
{
	if (!IsMasteryActive(Identity))
	{
		return;
	}

	ActiveMasteries.Remove(Identity);
	// Note: We do NOT clear UnlockedSkillsMap, SpentPointsMap, or TotalPointsMap.
	// The mastery record persists so it can be reactivated without losing progress.
	// Only MasteryDefinitions is cleared since the definition reference is no longer needed.
	MasteryDefinitions.Remove(Identity);

	OnMasteryDeactivated.Broadcast(Identity);
}

bool UKDMasteryComponent::TryUnlockSkill(FGameplayTag SkillTag, FKDMasteryIdentity Identity)
{
	// Validate eligibility
	if (!CanUnlockSkill(SkillTag, Identity))
	{
		return false;
	}

	// Find the slot to get the point cost
	UKDMasteryDefinition* Def = nullptr;
	{
		const TObjectPtr<UKDMasteryDefinition>* DefPtr = MasteryDefinitions.Find(Identity);
		if (!DefPtr || !*DefPtr)
		{
			return false;
		}
		Def = *DefPtr;
	}

	int32 Cost = 0;
	for (const FKDMasterySlot& Slot : Def->Slots)
	{
		if (!Slot.Skill.IsNull())
		{
			if (const UKDSkillDefinition* SkillDef = Slot.Skill.LoadSynchronous())
			{
				if (SkillDef->SkillTag == SkillTag)
				{
					Cost = SkillDef->MasteryPointCost;
					break;
				}
			}
		}
	}

	// Spend points
	int32& Spent = SpentPointsMap.FindOrAdd(Identity, 0);
	Spent += Cost;

	// Record unlocked skill
	TArray<FGameplayTag>& Skills = UnlockedSkillsMap.FindOrAdd(Identity, TArray<FGameplayTag>());
	Skills.Add(SkillTag);

	// Publish event (does NOT call or manage listeners)
	OnSkillUnlocked.Broadcast(SkillTag, Identity);

	return true;
}

void UKDMasteryComponent::AddMasteryPoints(FKDMasteryIdentity Identity, int32 Points)
{
	if (Points <= 0)
	{
		return;
	}

	int32& Total = TotalPointsMap.FindOrAdd(Identity, 0);
	Total += Points;
}

int32 UKDMasteryComponent::GetAvailablePoints(FKDMasteryIdentity Identity) const
{
	const int32 Total = TotalPointsMap.FindRef(Identity);
	const int32 Spent = SpentPointsMap.FindRef(Identity);
	return FMath::Max(0, Total - Spent);
}

// ========================================================================
// State Restoration
// ========================================================================

int32 UKDMasteryComponent::GetTotalPoints(FKDMasteryIdentity Identity) const
{
	const int32* Points = TotalPointsMap.Find(Identity);
	return Points ? *Points : 0;
}

void UKDMasteryComponent::RestoreMasteryState(FKDMasteryIdentity Identity, const TArray<FGameplayTag>& UnlockedSkills, int32 TotalPoints, int32 SpentPoints)
{
	UnlockedSkillsMap.Add(Identity, UnlockedSkills);
	TotalPointsMap.Add(Identity, TotalPoints);
	SpentPointsMap.Add(Identity, SpentPoints);
}
