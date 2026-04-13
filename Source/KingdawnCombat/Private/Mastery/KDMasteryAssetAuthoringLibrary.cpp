// Copyright Kingdawn. All Rights Reserved.

#include "Mastery/KDMasteryAssetAuthoringLibrary.h"
#include "Mastery/KDMasteryDefinition.h"
#include "Mastery/KDSkillDefinition.h"
#include "Abilities/GameplayAbility.h"

FKDMasterySlot UKDMasteryAssetAuthoringLibrary::MakeMasterySlot(
	FGameplayTag SlotTag,
	UKDSkillDefinition* SkillDefinition,
	const TArray<FGameplayTag>& PrerequisiteSlots,
	FVector2D TreePosition)
{
	FKDMasterySlot Slot;
	Slot.SlotTag = SlotTag;
	Slot.Skill = SkillDefinition;
	Slot.PrerequisiteSlots = PrerequisiteSlots;
	Slot.TreePosition = TreePosition;
	return Slot;
}

void UKDMasteryAssetAuthoringLibrary::ConfigureSkillDefinition(
	UKDSkillDefinition* SkillDefinition,
	FGameplayTag SkillTag,
	FText DisplayName,
	FText Description,
	EKDMasteryTier RequiredTier,
	int32 MasteryPointCost,
	TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!SkillDefinition)
	{
		return;
	}

	SkillDefinition->SkillTag = SkillTag;
	SkillDefinition->DisplayName = DisplayName;
	SkillDefinition->Description = Description;
	SkillDefinition->RequiredTier = RequiredTier;
	SkillDefinition->MasteryPointCost = MasteryPointCost;
	SkillDefinition->AbilityClass = AbilityClass;
}

void UKDMasteryAssetAuthoringLibrary::ConfigureMasteryDefinition(
	UKDMasteryDefinition* MasteryDefinition,
	EKDMasteryClass MasteryClass,
	EKDMasteryBranch MasteryBranch,
	FText DisplayName,
	FText Description,
	const TMap<EKDMasteryTier, int32>& TierUnlockThresholds,
	const TArray<FKDMasterySlot>& Slots)
{
	if (!MasteryDefinition)
	{
		return;
	}

	MasteryDefinition->Identity.MasteryClass = MasteryClass;
	MasteryDefinition->Identity.MasteryBranch = MasteryBranch;
	MasteryDefinition->DisplayName = DisplayName;
	MasteryDefinition->Description = Description;
	MasteryDefinition->TierUnlockThresholds = TierUnlockThresholds;
	MasteryDefinition->Slots = Slots;
}
