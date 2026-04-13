// Copyright Kingdawn. All Rights Reserved.

#include "Data/KDBuildDefinition.h"
#include "Core/KDBranchTypes.h"

UKDBuildDefinition::UKDBuildDefinition()
	: CombatClass(EKDCombatClass::None)
	, CombatBranch(EKDCombatBranch::None)
	, PrimaryWeaponType(EKDWeaponType::None)
	, OffhandWeaponType(EKDWeaponType::None)
{
}

void UKDBuildDefinition::SyncLegacyFromMastery()
{
	CombatClass = ConvertToLegacyClass(MasteryIdentity.MasteryClass);
	CombatBranch = ConvertToLegacyBranch(MasteryIdentity.MasteryBranch);
}

void UKDBuildDefinition::SyncMasteryFromLegacy()
{
	MasteryIdentity.MasteryClass = ConvertToMasteryClass(CombatClass);
	MasteryIdentity.MasteryBranch = ConvertToMasteryBranch(CombatBranch);
}
