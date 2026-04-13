// Copyright Kingdawn. All Rights Reserved.

#include "Data/KDAbilityDefinition.h"

UKDAbilityDefinition::UKDAbilityDefinition()
{
	DefinitionType = EKDAbilityDefinitionType::ActiveSkill;
	SkillTier = EKDSkillTier::Common;
	StaminaCost = 0.f;
	ManaCost = 0.f;
	CooldownDuration = 0.f;
	bCommitOnActivate = true;
	bEndOnMontageComplete = true;
	MontagePlayRate = 1.f;
	bRequiresTarget = false;
	MinRange = 0.f;
	MaxRange = 300.f;
	bAutoFaceTarget = false;
	FacingInterpSpeed = 0.f;
	bHotbarAssignable = true;
}

bool UKDAbilityDefinition::IsCompatibleWith(EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon) const
{
	return Compatibility.IsCompatible(Class, Branch, Weapon);
}

bool UKDAbilityDefinition::IsCompatibleWithMastery(const FKDMasteryIdentity& MasteryId, EKDWeaponType Weapon) const
{
	return Compatibility.IsCompatibleWithMastery(MasteryId, Weapon);
}
