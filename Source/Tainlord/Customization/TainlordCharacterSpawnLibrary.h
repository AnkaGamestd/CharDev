// Copyright Kingdawn. All Rights Reserved.
// Spawn integration: single authoritative apply point for profile → character appearance.
// Lives in Tainlord module, bridges KingdawnCombat character with customization system.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCharacterSpawnLibrary.generated.h"

class UTainlordCharacterAppearanceComponent;

// Forward declare KingdawnCombat mastery types (minimal header touch)
struct FKDMasteryIdentity;

/**
 * Static helper library for spawn-time profile → appearance application.
 *
 * This is the single authoritative runtime apply point.
 * Both spawn path and creation preview path converge here.
 *
 * Call chain:
 *   Spawn/Possess → ApplyProfileToCharacter() → component.ApplyAppearanceWithContext()
 *   Creation UI Preview → ApplyProfileToCharacter() → component.ApplyAppearanceWithContext()
 *
 * Per Arch.md: There must NOT be a separate preview-only appearance system.
 *
 * Mastery bridge (v1):
 *   ApplyProfileToCharacter() now also applies mastery selection to combat characters.
 *   SelectedMasteryId → FKDMasteryIdentity → character's mastery component.
 */
UCLASS()
class TAINLORD_API UTainlordCharacterSpawnLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Apply the active profile from the subsystem to a character's appearance component.
	 * This is the primary spawn integration point.
	 *
	 * @param Character The character to apply appearance to. Must have a UTainlordCharacterAppearanceComponent.
	 * @param WorldContext World context for subsystem access.
	 * @return True if appearance was applied successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Spawn", meta = (WorldContext = "WorldContext"))
	static bool ApplyActiveProfileToCharacter(AActor* Character, const UObject* WorldContext);

	/**
	 * Apply a specific profile to a character's appearance component.
	 * Used by both spawn path and creation preview.
	 * This is the single authoritative apply point.
	 *
	 * V1 Mastery Bridge: Also applies SelectedMasteryId to combat characters.
	 * - Maps mastery ID to FKDMasteryIdentity via TainlordCombatRuleLibrary
	 * - Sets mastery identity on KDCombatCharacter if present
	 * - Logs warnings if mastery ID is invalid or character has no combat component
	 *
	 * @param Character The character to apply appearance to. Must have a UTainlordCharacterAppearanceComponent.
	 * @param ProfileData The profile data to apply.
	 * @return True if appearance was applied successfully. Mastery application is best-effort.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Spawn")
	static bool ApplyProfileToCharacter(AActor* Character, const FTainlordProfileData& ProfileData);

	/**
	 * Find the appearance component on a character.
	 * Works with any actor that has a UTainlordCharacterAppearanceComponent.
	 * @param Character The character to search.
	 * @return The appearance component, or null if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Spawn")
	static UTainlordCharacterAppearanceComponent* GetAppearanceComponent(AActor* Character);

private:
	/**
	 * Apply mastery selection from profile to a combat character.
	 * This is a v1 pragmatic bridge - code-based mapping with safe fallbacks.
	 *
	 * @param Character The character to apply mastery to.
	 * @param MasteryId The mastery ID from FTainlordProfileData.SelectedMasteryId.
	 */
	static void ApplyMasteryToCharacter(AActor* Character, FName MasteryId);
};
