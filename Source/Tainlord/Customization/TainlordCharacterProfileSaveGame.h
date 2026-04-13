// Copyright Kingdawn. All Rights Reserved.
// Save game payload for character profile data.
// Per Arch.md: C++ owns save payload class for profile data.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCharacterProfileSaveGame.generated.h"

/**
 * Save game payload for character profile data.
 * Stores the serialized FTainlordProfileData for persistence.
 *
 * Per Arch.md:
 * - Save/load orchestration is a separate system
 * - Profile data stores player intent
 * - Runtime systems derive gameplay state from that profile
 */
UCLASS()
class TAINLORD_API UTainlordCharacterProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UTainlordCharacterProfileSaveGame();

	/** The character profile data to persist. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Profile")
	FTainlordProfileData ProfileData;

	/**
	 * Save slot name. Each character occupies one slot.
	 * Format: "Character_{SlotIndex}" or a unique player-defined name.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Profile")
	FString SlotName;

	/**
	 * User index. Typically 0 for single-player.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Profile")
	int32 UserIndex;

	// --- Static helpers ---

	/**
	 * Save a profile to disk.
	 * @param ProfileData The profile data to save.
	 * @param SlotName The slot name to save to.
	 * @param UserIndex The user index (typically 0).
	 * @return True if save succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Profile|Save")
	static bool SaveProfile(const FTainlordProfileData& InProfileData, const FString& InSlotName, int32 InUserIndex = 0);

	/**
	 * Load a profile from disk.
	 * @param SlotName The slot name to load from.
	 * @param UserIndex The user index (typically 0).
	 * @param OutProfileData The loaded profile data (output).
	 * @return True if load succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Profile|Save")
	static bool LoadProfile(const FString& InSlotName, int32 InUserIndex, FTainlordProfileData& OutProfileData);

	/**
	 * Check if a profile save exists.
	 * @param SlotName The slot name to check.
	 * @param UserIndex The user index (typically 0).
	 * @return True if the save exists.
	 */
	UFUNCTION(BlueprintPure, Category = "Profile|Save")
	static bool DoesSaveExist(const FString& InSlotName, int32 InUserIndex = 0);

	/**
	 * Delete a profile save.
	 * @param SlotName The slot name to delete.
	 * @param UserIndex The user index (typically 0).
	 * @return True if the save was deleted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Profile|Save")
	static bool DeleteProfile(const FString& InSlotName, int32 InUserIndex = 0);
};
