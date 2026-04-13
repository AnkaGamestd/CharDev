// Copyright Kingdawn. All Rights Reserved.
// Build definition runtime overrides for testing.
// Separated from main character file for clean organization.

#include "Characters/KDCombatCharacter.h"
#include "Data/KDBuildDefinition.h"

// Console variable: test build override
// Allows switching between builds without editing Blueprint defaults
// 0 = Use Blueprint SelectedBuildDefinition (default)
// 1 = Force BladeShield build
// 2 = Force Wizard build
static TAutoConsoleVariable<int32> CVarTestBuild(
	TEXT("kd.build.testoverride"),
#if WITH_EDITOR
	0,  // Default: use Blueprint setting
#else
	0,
#endif
	TEXT("Override build definition for PIE testing.\n")
	TEXT("0 = Use Blueprint SelectedBuildDefinition (default)\n")
	TEXT("1 = Force BladeShield build (DA_Build_BladeShield)\n")
	TEXT("2 = Force Wizard build (DA_Build_Wizard)"),
	ECVF_Default
);

UKDBuildDefinition* AKDCombatCharacter::GetTestBuildOverride() const
{
	const int32 OverrideMode = CVarTestBuild.GetValueOnGameThread();
	
	if (OverrideMode == 0)
	{
		// No override - use Blueprint default
		return nullptr;
	}

	const TCHAR* BuildPath = nullptr;
	
	switch (OverrideMode)
	{
	case 1:
		BuildPath = TEXT("/Game/KingdawnCombat/Data/Builds/DA_Build_BladeShield.DA_Build_BladeShield");
		break;
	case 2:
		BuildPath = TEXT("/Game/KingdawnCombat/Data/Builds/DA_Build_Wizard.DA_Build_Wizard");
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: Invalid kd.build.testoverride value: %d"), OverrideMode);
		return nullptr;
	}

	UKDBuildDefinition* Build = Cast<UKDBuildDefinition>(
		FSoftObjectPath(BuildPath).TryLoad());

	if (!Build)
	{
		UE_LOG(LogTemp, Error, TEXT("KDCombatCharacter: Failed to load test build override from: %s"), BuildPath);
		return nullptr;
	}

	UE_LOG(LogTemp, Display, TEXT("KDCombatCharacter: Test build override active - loaded '%s'"), *Build->GetName());
	return Build;
}
