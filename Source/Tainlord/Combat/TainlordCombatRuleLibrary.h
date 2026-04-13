#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Domain/TainlordGameplayTypes.h"
#include "Core/KDMasteryTypes.h"  // Full include for FKDMasteryIdentity (UHT requirement)
#include "TainlordCombatRuleLibrary.generated.h"

UCLASS()
class TAINLORD_API UTainlordCombatRuleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// --- Existing combat rule functions ---
	
	UFUNCTION(BlueprintPure, Category = "Tainlord|Combat")
	static FTainlordStatBlock BuildDerivedStats(const FTainlordCharacterState& CharacterState);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Combat")
	static FTainlordCombatResources BuildCombatResources(const FTainlordCharacterState& CharacterState);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Combat")
	static FTainlordCombatBudget GetStarterCombatBudget(ETainlordArchetype Archetype);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Combat")
	static FTainlordCombatResolution ResolveActionMagnitude(const FTainlordStatBlock& AttackerStats, const FTainlordAttackProfile& ActionProfile, int32 TargetDefense);

	// --- Mastery bridge functions (v1: code-based mapping) ---
	
	/**
	 * Map a Tainlord mastery ID to KingdawnCombat's FKDMasteryIdentity.
	 * This is a pragmatic v1 bridge using code-based mapping.
	 *
	 * Demo mastery mappings:
	 *   "Berserker"   -> (Warrior, BladeShield)
	 *   "Guardian"    -> (Warrior, BladeShield)
	 *   "Arcanist"    -> (Mage, Wizard)
	 *   "Cleric"      -> (Support, Cleric)
	 *   "Ranger"      -> (Archer, Archer)
	 *   "Shadowblade" -> (Rogue, Dagger)
	 *
	 * @param MasteryId The Tainlord mastery ID from FTainlordProfileData.SelectedMasteryId
	 * @return FKDMasteryIdentity for runtime combat/mastery systems. Invalid if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Tainlord|Mastery|Bridge")
	static FKDMasteryIdentity MapMasteryIdToCombatIdentity(FName MasteryId);

	/**
	 * Check if a mastery ID is valid for the demo build.
	 * Uses the same mapping as MapMasteryIdToCombatIdentity.
	 *
	 * @param MasteryId The mastery ID to validate
	 * @return True if this mastery ID is recognized and has a valid combat mapping
	 */
	UFUNCTION(BlueprintPure, Category = "Tainlord|Mastery|Bridge")
	static bool IsValidDemoMasteryId(FName MasteryId);

	/**
	 * Get the ETainlordArchetype for a given mastery ID.
	 * Used for stat derivation when no full mastery catalog is available.
	 *
	 * @param MasteryId The mastery ID to query
	 * @return ETainlordArchetype for this mastery. Returns Warrior as safe fallback.
	 */
	UFUNCTION(BlueprintPure, Category = "Tainlord|Mastery|Bridge")
	static ETainlordArchetype GetMasteryArchetype(FName MasteryId);

	/**
	 * Get the ETainlordStatOrientation for a given mastery ID.
	 * Used for stat derivation when no full mastery catalog is available.
	 *
	 * @param MasteryId The mastery ID to query
	 * @return ETainlordStatOrientation for this mastery. Returns Strength as safe fallback.
	 */
	UFUNCTION(BlueprintPure, Category = "Tainlord|Mastery|Bridge")
	static ETainlordStatOrientation GetMasteryStatOrientation(FName MasteryId);
};
