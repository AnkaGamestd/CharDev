// Copyright Kingdawn. All Rights Reserved.

#include "Mastery/KDMasteryTestBootstrap.h"
#include "Mastery/KDMasteryComponent.h"
#include "Mastery/KDMasteryDefinition.h"
#include "Mastery/KDSkillDefinition.h"
#include "Mastery/KDMasterySlot.h"
#include "Components/KDAbilitySystemComponent.h"
#include "Components/KDHotbarComponent.h"
#include "Characters/KDCombatCharacter.h"
#include "Core/KDMasteryTypes.h"
#include "Core/KDGameplayTags.h"
#include "GameFramework/PlayerController.h"

// Console variable: test mode flag
// Default: 0 (disabled)
// Set to 1 to enable test bootstrap during PIE
static TAutoConsoleVariable<int32> CVarMasteryTestMode(
	TEXT("kd.mastery.testmode"),
#if WITH_EDITOR
	1,
#else
	0,
#endif
	TEXT("Enable mastery test bootstrap for PIE testing.\n")
	TEXT("0 = Disabled (default, production mode)\n")
	TEXT("1 = Enabled (test mode, auto-setup mastery on character spawn)"),
	ECVF_Default
);

// ========================================================================
// Expected asset paths for real authored mastery data assets.
// When these .uasset files exist in the Content Browser, the bootstrap
// loads them directly. When they do not exist, the bootstrap creates
// transient runtime equivalents and logs a clear migration warning.
//
// To author these assets:
//   1. Run the Python script: Content/KingdawnCombat/Scripts/setup_mastery_assets.py
//      OR create them manually in the Content Browser
//   2. Verify the assets are at these exact paths
//   3. The bootstrap will automatically detect and use them
// ========================================================================
static const TCHAR* ASSET_PATH_MASTERY_DEF =
	TEXT("/Game/KingdawnCombat/Data/Data/Mastery/DA_Mastery_Wizard.DA_Mastery_Wizard");

// Skill and ability class paths are no longer needed here.
// The authored mastery definition (MD_Warrior_BladeShield) contains
// all skill definitions and their ability class references internally.

bool FKDMasteryTestBootstrap::IsTestModeEnabled()
{
	return CVarMasteryTestMode.GetValueOnGameThread() > 0;
}

bool FKDMasteryTestBootstrap::RunTestBootstrap(AKDCombatCharacter* Character)
{
	// Gate: test mode must be enabled
	if (!IsTestModeEnabled())
	{
		return false;
	}

	// Gate: character must be valid
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDMasteryTestBootstrap: Character is null, cannot run bootstrap"));
		return false;
	}

	// Gate: only run on player-controlled characters (not AI/NPCs)
	if (!Character->IsPlayerControlled())
	{
		return false;
	}

	UE_LOG(LogTemp, Display, TEXT("=== KDMasteryTestBootstrap: Starting test bootstrap for %s ==="), *Character->GetName());

	// Step 1: Find mastery component
	UKDMasteryComponent* MasteryComp = Character->FindComponentByClass<UKDMasteryComponent>();
	if (!MasteryComp)
	{
		UE_LOG(LogTemp, Error, TEXT("KDMasteryTestBootstrap: No KDMasteryComponent found on character, cannot bootstrap"));
		return false;
	}

	// Step 2: Create test mastery definition
	UKDMasteryDefinition* TestMasteryDef = CreateTestMasteryDefinition();
	if (!TestMasteryDef)
	{
		UE_LOG(LogTemp, Error, TEXT("KDMasteryTestBootstrap: Failed to create test mastery definition"));
		return false;
	}

	const FKDMasteryIdentity TestIdentity = TestMasteryDef->Identity;
	UE_LOG(LogTemp, Display, TEXT("KDMasteryTestBootstrap: Activating test mastery: %s / %s"),
		*UEnum::GetValueAsString(TestIdentity.MasteryClass),
		*UEnum::GetValueAsString(TestIdentity.MasteryBranch));

	// Mastery component activation alone does not update entity-side compatibility.
	// Keep the character's primary mastery identity in sync before any ability checks run.
	Character->SetMasteryIdentity(TestIdentity);

	// Step 3: Activate mastery
	MasteryComp->ActivateMastery(TestIdentity, TestMasteryDef);

	// Step 4: Add test mastery points (enough to unlock tier 1 skills)
	const int32 TestPoints = 10;
	MasteryComp->AddMasteryPoints(TestIdentity, TestPoints);
	UE_LOG(LogTemp, Display, TEXT("KDMasteryTestBootstrap: Added %d mastery points"), TestPoints);

	// Step 5: Unlock test skills
	TArray<FGameplayTag> SkillsToUnlock;
	
	// Find tier 1 skills (root skills with no prerequisites)
	for (const FKDMasterySlot& Slot : TestMasteryDef->Slots)
	{
		if (Slot.PrerequisiteSlots.Num() == 0 && !Slot.Skill.IsNull())
		{
			if (UKDSkillDefinition* SkillDef = Slot.Skill.LoadSynchronous())
			{
				if (SkillDef->SkillTag.IsValid())
				{
					SkillsToUnlock.Add(SkillDef->SkillTag);
				}
			}
		}

		// Unlock max 2 skills for first playable test
		if (SkillsToUnlock.Num() >= 2)
		{
			break;
		}
	}

	// Unlock the skills via mastery component (respects architecture boundary)
	for (const FGameplayTag& SkillTag : SkillsToUnlock)
	{
		const bool bUnlocked = MasteryComp->TryUnlockSkill(SkillTag, TestIdentity);
		if (bUnlocked)
		{
			UE_LOG(LogTemp, Display, TEXT("KDMasteryTestBootstrap: Unlocked skill: %s"), *SkillTag.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("KDMasteryTestBootstrap: Failed to unlock skill: %s"), *SkillTag.ToString());
		}
	}

	// Step 6: ASC mastery reconciliation
	UKDAbilitySystemComponent* ASC = Character->GetKDAbilitySystemComponent();
	if (ASC)
	{
		UE_LOG(LogTemp, Display, TEXT("KDMasteryTestBootstrap: Running ASC mastery reconciliation..."));
		const FKDMasteryReconciliationResult ASCResult = ASC->ReconcileMasteryGrantedAbilities();
		
		UE_LOG(LogTemp, Display, TEXT("KDMasteryTestBootstrap: ASC reconciliation complete - Granted: %d, Revoked: %d, Failed: %d"),
			ASCResult.GrantedSkills.Num(),
			ASCResult.RevokedSkills.Num(),
			ASCResult.FailedSkills.Num());

		for (const FGameplayTag& GrantedTag : ASCResult.GrantedSkills)
		{
			UE_LOG(LogTemp, Display, TEXT("  - Granted ability: %s"), *GrantedTag.ToString());
		}

		for (const FGameplayTag& FailedTag : ASCResult.FailedSkills)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - Failed to grant: %s"), *FailedTag.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDMasteryTestBootstrap: No ASC found, skipping ability reconciliation"));
	}

	// Step 7: Hotbar mastery reconciliation
	UKDHotbarComponent* Hotbar = nullptr;
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		Hotbar = PC->FindComponentByClass<UKDHotbarComponent>();
	}

	if (Hotbar)
	{
		UE_LOG(LogTemp, Display, TEXT("KDMasteryTestBootstrap: Running hotbar mastery reconciliation..."));
		const int32 PlacedCount = Hotbar->ReconcileMasteryHotbar();
		UE_LOG(LogTemp, Display, TEXT("KDMasteryTestBootstrap: Hotbar reconciliation complete - %d slots modified"), PlacedCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDMasteryTestBootstrap: No hotbar component found, skipping hotbar reconciliation"));
	}

	// Step 8: Log summary
	LogBootstrapSummary(MasteryComp, ASC, Hotbar);

	UE_LOG(LogTemp, Display, TEXT("=== KDMasteryTestBootstrap: Bootstrap complete ==="));
	return true;
}

UKDMasteryDefinition* FKDMasteryTestBootstrap::CreateTestMasteryDefinition()
{
	// ================================================================
	// AUTHORITY PATH: Load real authored data assets only.
	// No transient fallbacks. If the authored mastery definition
	// does not exist, the bootstrap fails loudly with an Error log.
	// ================================================================

	UKDMasteryDefinition* AuthoredDef = Cast<UKDMasteryDefinition>(
		FSoftObjectPath(ASSET_PATH_MASTERY_DEF).TryLoad());

	if (AuthoredDef)
	{
		UE_LOG(LogTemp, Display,
			TEXT("KDMasteryTestBootstrap: Loaded REAL authored mastery definition: %s"),
			ASSET_PATH_MASTERY_DEF);
		return AuthoredDef;
	}

	// ================================================================
	// FALLBACK: Authored assets not yet created.
	// Create transient equivalents using REAL BladeShield ability classes.
	//
	// MIGRATION WARNING:
	// This fallback creates transient UKDSkillDefinition objects that
	// will NOT persist across sessions. To eliminate this fallback:
	//   1. Author UKDSkillDefinition assets at:
	//      /Game/KingdawnCombat/Mastery/Skills/SD_BladeShield_HeavyStrike
	//      /Game/KingdawnCombat/Mastery/Skills/SD_BladeShield_ShieldBash
	//   2. Author UKDMasteryDefinition asset at:
	//      /Game/KingdawnCombat/Mastery/Definitions/MD_Warrior_BladeShield
	//   3. Run setup_mastery_assets.py in the UE Python console to scaffold
	//   4. Once authored, this fallback code path becomes dead code
	// ================================================================

	UE_LOG(LogTemp, Error,
		TEXT("KDMasteryTestBootstrap: REQUIRED authored mastery definition not found at %s. ")
		TEXT("Bootstrap aborted. Author the asset and retry."),
		ASSET_PATH_MASTERY_DEF);

	return nullptr;
}

void FKDMasteryTestBootstrap::LogBootstrapSummary(
	UKDMasteryComponent* MasteryComp,
	UKDAbilitySystemComponent* ASC,
	UKDHotbarComponent* Hotbar)
{
	if (!MasteryComp)
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Mastery Test Bootstrap Summary ---"));

	// Mastery state
	const TArray<FKDMasteryIdentity>& ActiveMasteries = MasteryComp->GetActiveMasteries();
	UE_LOG(LogTemp, Display, TEXT("Active Masteries: %d"), ActiveMasteries.Num());
	for (const FKDMasteryIdentity& Identity : ActiveMasteries)
	{
		const int32 Spent = MasteryComp->GetSpentPoints(Identity);
		const int32 Available = MasteryComp->GetAvailablePoints(Identity);
		const TArray<FGameplayTag> UnlockedSkills = MasteryComp->GetUnlockedSkills(Identity);

		UE_LOG(LogTemp, Display, TEXT("  %s / %s - Points: %d spent, %d available, %d skills unlocked"),
			*UEnum::GetValueAsString(Identity.MasteryClass),
			*UEnum::GetValueAsString(Identity.MasteryBranch),
			Spent,
			Available,
			UnlockedSkills.Num());

		for (const FGameplayTag& SkillTag : UnlockedSkills)
		{
			UE_LOG(LogTemp, Display, TEXT("    - %s"), *SkillTag.ToString());
		}
	}

	// ASC granted abilities
	if (ASC)
	{
		const TSet<FGameplayTag>& MasteryGrantedSkills = ASC->GetMasteryGrantedSkills();
		UE_LOG(LogTemp, Display, TEXT("Mastery-Granted Abilities: %d"), MasteryGrantedSkills.Num());
		for (const FGameplayTag& SkillTag : MasteryGrantedSkills)
		{
			UE_LOG(LogTemp, Display, TEXT("  - %s"), *SkillTag.ToString());
		}
	}

	// Hotbar placed skills
	if (Hotbar)
	{
		int32 TotalPlacedCount = 0;
		for (int32 BarIdx = 0; BarIdx < Hotbar->GetBarCount(); ++BarIdx)
		{
			const FKDHotbarBar& Bar = Hotbar->GetBar(BarIdx);
			for (int32 SlotIdx = 0; SlotIdx < Bar.Slots.Num(); ++SlotIdx)
			{
				if (Bar.Slots[SlotIdx].IsAssigned() && Hotbar->IsMasteryPlacedSkill(Bar.Slots[SlotIdx].AbilityTag))
				{
					TotalPlacedCount++;
					UE_LOG(LogTemp, Display, TEXT("Hotbar Slot [Bar %d, Slot %d]: %s (mastery-placed)"),
						BarIdx, SlotIdx, *Bar.Slots[SlotIdx].AbilityTag.ToString());
				}
			}
		}
		UE_LOG(LogTemp, Display, TEXT("Mastery-Placed Hotbar Slots: %d"), TotalPlacedCount);
	}

	UE_LOG(LogTemp, Display, TEXT("--------------------------------------"));
	UE_LOG(LogTemp, Display, TEXT(""));
}


