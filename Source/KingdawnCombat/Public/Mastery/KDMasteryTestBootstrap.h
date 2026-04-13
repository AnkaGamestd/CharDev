// Copyright Kingdawn. All Rights Reserved.
// Mastery test bootstrap: programmatic mastery setup for PIE testing.
//
// PURPOSE:
// Enable first playable mastery test without manual Blueprint graph wiring.
// This is a CLEAN TEST HARNESS, not a hacky throwaway.
//
// ARCHITECTURE COMPLIANCE:
// - Respects Arch.md boundaries: does not move unlock/grant/hotbar ownership
// - KDMasteryComponent owns unlock state
// - KDAbilitySystemComponent owns ability grants (via reconciliation)
// - KDHotbarComponent owns hotbar placement (via reconciliation)
// - Bootstrap orchestrates the flow but does NOT own the logic
//
// TEST FLAG:
// Only runs when console variable `kd.mastery.testmode 1` is set.
// Default is 0 (disabled). Not a production default.
//
// REMOVAL:
// This file can be deleted cleanly once content-driven mastery setup is ready.
// No production code depends on this test harness.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class AKDCombatCharacter;
class UKDMasteryComponent;
class UKDAbilitySystemComponent;
class UKDHotbarComponent;
class UKDMasteryDefinition;

/**
 * Mastery test bootstrap utility.
 * 
 * Provides a programmatic path to set up mastery state for PIE testing
 * without requiring manual Blueprint event graph wiring.
 * 
 * This is NOT a component or subsystem. It's a stateless utility class
 * that runs once during character initialization when test mode is enabled.
 */
class KINGDAWNCOMBAT_API FKDMasteryTestBootstrap
{
public:
	/**
	 * Check if mastery test mode is enabled.
	 * Reads the console variable `kd.mastery.testmode`.
	 * 
	 * @return True if test mode is enabled (CVar > 0)
	 */
	static bool IsTestModeEnabled();

	/**
	 * Run the mastery test bootstrap flow on a character.
	 * 
	 * Flow:
	 * 1. Check test mode flag (exit early if disabled)
	 * 2. Find KDMasteryComponent on the actor
	 * 3. Activate a test mastery definition (hardcoded for first playable)
	 * 4. Add test mastery points
	 * 5. Unlock test skills in code
	 * 6. Call ASC mastery reconciliation (grants abilities)
	 * 7. Call hotbar mastery reconciliation (places skills on hotbar)
	 * 8. Log a clean summary
	 * 
	 * IMPORTANT:
	 * - This does NOT bypass normal mastery rules
	 * - This does NOT grant abilities directly
	 * - This does NOT place hotbar slots directly
	 * - This ONLY sets up mastery state, then delegates to reconciliation
	 * 
	 * @param Character The character to bootstrap (must be valid, must have mastery component)
	 * @return True if bootstrap succeeded
	 */
	static bool RunTestBootstrap(AKDCombatCharacter* Character);

private:
	// No instances - static utility only
	FKDMasteryTestBootstrap() = delete;

	/**
	 * Create or load the test mastery definition.
	 * 
	 * For first playable, this returns a hardcoded test definition:
	 * - Identity: Warrior / BladeShield
	 * - Contains 2-3 test skills
	 * - Tier 1 unlocks at 0 points (root skills)
	 * 
	 * In the future, this could load from a real data asset.
	 * For now, it constructs a minimal test definition in code.
	 * 
	 * @return A test mastery definition (always non-null)
	 */
	static UKDMasteryDefinition* CreateTestMasteryDefinition();

	/**
	 * Log a bootstrap summary to the console.
	 * Reports what was activated, unlocked, granted, and placed.
	 * 
	 * @param MasteryComp The mastery component that was modified
	 * @param ASC The ability system component (for grant report)
	 * @param Hotbar The hotbar component (for placement report)
	 */
	static void LogBootstrapSummary(
		UKDMasteryComponent* MasteryComp,
		UKDAbilitySystemComponent* ASC,
		UKDHotbarComponent* Hotbar);
};
