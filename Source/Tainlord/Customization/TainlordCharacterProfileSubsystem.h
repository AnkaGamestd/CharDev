// Copyright Kingdawn. All Rights Reserved.
// Game Instance Subsystem for character profile management.
// Owns the active profile and orchestrates load/save.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCharacterProfileSubsystem.generated.h"

/**
 * Delegate fired when character creation flow should be triggered.
 * Broadcast when no save exists and the player needs to create a character.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreationFlowNeeded);

/**
 * Game Instance Subsystem for character profile management.
 *
 * Responsibilities:
 * - Load profile on game start (or trigger creation flow if none exists)
 * - Hold the active profile data for runtime systems to query
 * - Save profile on exit or explicit save request
 *
 * Per Arch.md:
 * - This subsystem owns profile orchestration only
 * - Appearance apply logic lives in UTainlordCharacterAppearanceComponent
 * - Build/mastery derivation lives in respective systems
 */
UCLASS()
class TAINLORD_API UTainlordCharacterProfileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// --- Subsystem lifecycle ---
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Creation flow hook ---

	/**
	 * Delegate broadcast when no profile save exists and character creation flow should be triggered.
	 * Blueprint and C++ can bind to this to open creation UI.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
	FOnCreationFlowNeeded OnCreationFlowNeeded;

	// --- Profile access ---

	/**
	 * Get the currently loaded profile.
	 * Returns an empty profile if none is loaded.
	 */
	UFUNCTION(BlueprintPure, Category = "Profile")
	const FTainlordProfileData& GetActiveProfile() const { return ActiveProfile; }

	/**
	 * Check if a profile is currently loaded.
	 * Returns false if the active profile is empty/default.
	 */
	UFUNCTION(BlueprintPure, Category = "Profile")
	bool HasActiveProfile() const;

	// --- Profile load/save ---

	/**
	 * Load a profile from the given slot.
	 * If load succeeds, the profile becomes the active profile.
	 * @param SlotName The slot name to load from (e.g. "Character_0").
	 * @param UserIndex The user index (typically 0).
	 * @return True if load succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Profile")
	bool LoadProfileFromSlot(const FString& SlotName, int32 UserIndex = 0);

	/**
	 * Save the active profile to the given slot.
	 * @param SlotName The slot name to save to (e.g. "Character_0").
	 * @param UserIndex The user index (typically 0).
	 * @return True if save succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Profile")
	bool SaveActiveProfile(const FString& SlotName, int32 UserIndex = 0);

	/**
	 * Set the active profile directly (e.g. after creation UI).
	 * This does NOT save automatically — call SaveActiveProfile() to persist.
	 */
	UFUNCTION(BlueprintCallable, Category = "Profile")
	void SetActiveProfile(const FTainlordProfileData& NewProfile);

	/**
	 * Clear the active profile (e.g. logout, return to main menu).
	 */
	UFUNCTION(BlueprintCallable, Category = "Profile")
	void ClearActiveProfile();

	// --- Auto-load hook ---

	/**
	 * Attempt to auto-load a profile on subsystem init.
	 * Checks if a default slot exists and loads it.
	 * If no profile exists, ActiveProfile remains empty.
	 * Future: trigger creation UI flow here.
	 */
	void AttemptAutoLoad();

	// --- Default slot config ---

	/** Default slot name for auto-load. Can be overridden via project settings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Config")
	FString DefaultSlotName = TEXT("Character_0");

	/** Default user index for save/load. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Config")
	int32 DefaultUserIndex = 0;

private:
	/** The currently active character profile. */
	UPROPERTY(Transient)
	FTainlordProfileData ActiveProfile;
};
