// Copyright Kingdawn. All Rights Reserved.

#include "Mastery/KDMasteryDefinition.h"

UKDMasteryDefinition::UKDMasteryDefinition()
{
	// Default tier thresholds: tier 1 is free, higher tiers cost more
	TierUnlockThresholds.Add(EKDMasteryTier::Tier1, 0);
	TierUnlockThresholds.Add(EKDMasteryTier::Tier2, 5);
	TierUnlockThresholds.Add(EKDMasteryTier::Tier3, 15);
	TierUnlockThresholds.Add(EKDMasteryTier::Tier4, 30);
}

const FKDMasterySlot* UKDMasteryDefinition::FindSlotByTag(FGameplayTag SlotTag) const
{
	if (!SlotTag.IsValid())
	{
		return nullptr;
	}

	for (const FKDMasterySlot& Slot : Slots)
	{
		if (Slot.SlotTag == SlotTag)
		{
			return &Slot;
		}
	}

	return nullptr;
}

bool UKDMasteryDefinition::GetSlotByTag(FGameplayTag SlotTag, FKDMasterySlot& OutSlot) const
{
	const FKDMasterySlot* Found = FindSlotByTag(SlotTag);
	if (Found)
	{
		OutSlot = *Found;
		return true;
	}
	return false;
}

bool UKDMasteryDefinition::IsTierUnlocked(EKDMasteryTier Tier, int32 SpentPoints) const
{
	const int32* Threshold = TierUnlockThresholds.Find(Tier);
	if (!Threshold)
	{
		// Tier not configured: default to locked
		return false;
	}
	return SpentPoints >= *Threshold;
}

EKDMasteryTier UKDMasteryDefinition::GetHighestUnlockedTier(int32 SpentPoints) const
{
	EKDMasteryTier Highest = EKDMasteryTier::Tier1;

	// Walk tiers in order. Since TMap is unordered, iterate all and pick highest.
	static const EKDMasteryTier TierOrder[] = {
		EKDMasteryTier::Tier1,
		EKDMasteryTier::Tier2,
		EKDMasteryTier::Tier3,
		EKDMasteryTier::Tier4
	};

	for (EKDMasteryTier Tier : TierOrder)
	{
		if (IsTierUnlocked(Tier, SpentPoints))
		{
			Highest = Tier;
		}
	}

	return Highest;
}
