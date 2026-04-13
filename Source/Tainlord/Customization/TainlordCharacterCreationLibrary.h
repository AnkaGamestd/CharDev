// Copyright Kingdawn. All Rights Reserved.
// C++ API for character creation UI.
// All creation logic is C++ — UI (WBP) calls this library, makes no decisions.
// Preview uses the same ApplyProfileToCharacter path as spawn.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCharacterCreationLibrary.generated.h"

class UTainlordCharacterCustomizationCatalog;
class UTainlordCharacterAppearanceComponent;

/**
 * Static helper library providing the C++ API for the character creation flow.
 *
 * Design rules (per Arch.md):
 * - UI makes no decisions — it calls this API.
 * - Preview uses the same ApplyProfileToCharacter path as spawn (no duplicate logic).
 * - AppearanceComponent touches visuals only.
 * - This library orchestrates filtering, preview, validation, and confirm/save.
 *
 * Call chain:
 *   WBP_CharacterCreation → CreationLibrary → SpawnLibrary / ProfileSubsystem
 */
UCLASS()
class TAINLORD_API UTainlordCharacterCreationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// --- Filtering queries ---

	/**
	 * Get all head options available for the given gender and race.
	 * UI populates its head selector from this.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Filtering", meta = (WorldContext = "WorldContext"))
	static TArray<FTainlordHeadEntry> GetAvailableHeads(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race);

	/**
	 * Get all hair options available for the given gender and race.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Filtering", meta = (WorldContext = "WorldContext"))
	static TArray<FTainlordHairEntry> GetAvailableHair(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race);

	/**
	 * Get all beard options available for the given gender and race.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Filtering", meta = (WorldContext = "WorldContext"))
	static TArray<FTainlordBeardEntry> GetAvailableBeards(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race);

	/**
	 * Get all skin tone options. Skin tones are not gender/race gated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Filtering", meta = (WorldContext = "WorldContext"))
	static TArray<FTainlordSkinToneEntry> GetAvailableSkinTones(const UObject* WorldContext);

	// --- Live preview ---

	/**
	 * Apply a work-in-progress profile to the preview character.
	 * Uses the SAME apply path as spawn: UTainlordCharacterSpawnLibrary::ApplyProfileToCharacter.
	 * No duplicate preview logic — per Arch.md shared apply path.
	 *
	 * @param PreviewCharacter The preview character actor (must have UTainlordCharacterAppearanceComponent).
	 * @param WorkingProfile The current creation state to preview.
	 * @return True if preview was applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Preview")
	static bool ApplyPreview(AActor* PreviewCharacter, const FTainlordProfileData& WorkingProfile);

	/**
	 * Apply a single appearance change to the preview character without rebuilding the entire profile.
	 * Sets the profile context first, then applies the individual slot.
	 *
	 * @param PreviewCharacter The preview character actor.
	 * @param WorkingProfile The full working profile (used for gender/race context).
	 * @param SlotName Which slot to update: "Head", "Hair", "Beard", or "SkinTone".
	 * @param NewId The new stable ID to apply for this slot.
	 * @return True if the slot was applied successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Preview")
	static bool ApplyPreviewSlot(AActor* PreviewCharacter, const FTainlordProfileData& WorkingProfile, FName SlotName, FName NewId);

	// --- Validation ---

	/**
	 * Validate that a profile has the minimum required data for creation.
	 * Checks: name is not empty, gender is not Any, race is not Any.
	 * This is the legacy full-validation entry point (equivalent to ValidateProfileForStage with Confirm).
	 *
	 * @param Profile The profile to validate.
	 * @param OutReason Human-readable reason if validation fails.
	 * @return True if the profile is valid for creation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Validation")
	static bool ValidateProfile(const FTainlordProfileData& Profile, FString& OutReason);

	/**
	 * Stage-gated validation for the creation flow.
	 * Each stage checks only the requirements for that stage and all previous stages.
	 * UI calls this before allowing the player to advance to the next stage.
	 *
	 * Stage validation order:
	 *   RaceSelect: Gender != Any, Race != Any
	 *   Appearance: (no mandatory checks — appearance is optional)
	 *   Mastery:    SelectedMasteryId != None (if stage is reached)
	 *   City:       SelectedCityId != None (if stage is reached)
	 *   Name:       CharacterName is valid (2-32 chars, not empty)
	 *   Confirm:    All of the above
	 *
	 * @param Profile The working profile to validate.
	 * @param Stage   Which creation stage to validate up to.
	 * @return Structured result with success flag, reason, and failed stage.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Validation")
	static FProfileValidationResult ValidateProfileForStage(const FTainlordProfileData& Profile, ECreationStage Stage);

	/**
	 * Check if a profile is ready for final confirmation.
	 * Convenience wrapper — equivalent to ValidateProfileForStage(Confirm).
	 *
	 * @param Profile The profile to validate.
	 * @return Structured result with success flag, reason, and failed stage.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Validation")
	static FProfileValidationResult ValidateForConfirm(const FTainlordProfileData& Profile);

	// --- Confirm / save ---

	/**
	 * Confirm the creation: validate, set as active profile in the subsystem, and save.
	 * This is the single entry point for completing character creation.
	 *
	 * @param WorldContext World context for subsystem access.
	 * @param FinalProfile The completed profile to commit.
	 * @param OutReason Human-readable reason if confirmation fails.
	 * @return True if profile was committed and saved successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Confirm", meta = (WorldContext = "WorldContext"))
	static bool ConfirmCreation(const UObject* WorldContext, const FTainlordProfileData& FinalProfile, FString& OutReason);

	// --- Context-aware re-filter ---

	/**
	 * When gender or race changes, the current appearance selections may become invalid.
	 * This function checks each appearance slot against the new context and clears
	 * any selection that would be rejected by the catalog filter.
	 *
	 * Call this after the player changes gender or race in the UI, then re-apply preview.
	 *
	 * @param WorldContext World context for catalog access.
	 * @param InOutProfile The working profile to sanitize. Slots that don't match the new context are set to NAME_None.
	 * @return True if any slots were cleared (UI should refresh those selectors).
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Filtering", meta = (WorldContext = "WorldContext"))
	static bool SanitizeAppearanceForContext(const UObject* WorldContext, UPARAM(ref) FTainlordProfileData& InOutProfile);

private:
	/**
	 * Load the catalog from the default catalog reference or the subsystem's catalog.
	 * Returns null if no catalog can be found.
	 */
	static UTainlordCharacterCustomizationCatalog* FindCatalog(const UObject* WorldContext);
};
