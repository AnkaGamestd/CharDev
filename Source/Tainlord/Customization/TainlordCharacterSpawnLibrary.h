// Copyright Kingdawn. All Rights Reserved.
// Spawn integration: single authoritative apply point for profile → character appearance.
// Lives in Tainlord module, bridges KingdawnCombat character with customization system.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCharacterSpawnLibrary.generated.h"

class UTainlordCharacterAppearanceComponent;

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
	 * @param Character The character to apply appearance to. Must have a UTainlordCharacterAppearanceComponent.
	 * @param ProfileData The profile data to apply.
	 * @return True if appearance was applied successfully.
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
};
