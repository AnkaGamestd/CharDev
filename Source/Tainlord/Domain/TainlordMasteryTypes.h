// Copyright Kingdawn. All Rights Reserved.
// Mastery domain types — character build selection and derivation.
// Mastery determines archetype, stat orientation, starter skills, and identity.
// This is a SKELETON CONTRACT for future implementation — no UI, no full catalog yet.

#pragma once

#include "CoreMinimal.h"
#include "TainlordGameplayTypes.h"
#include "TainlordMasteryTypes.generated.h"

// ---------------------------------------------------------------------------
// Mastery definition
// ---------------------------------------------------------------------------

/**
 * A single mastery/build option in the character creation flow.
 * Each mastery represents a distinct playstyle identity.
 *
 * Per product design:
 * - Player picks ONE mastery during character creation (Stage 3)
 * - Mastery derives archetype, stat orientation, starter skills
 * - This struct is the stable-ID catalog entry contract
 *
 * Future extension points:
 * - Add TSoftObjectPtr<UTexture2D> IconAsset for UI preview
 * - Add FText FlavorDescription for narrative identity
 * - Add TArray<FName> StarterSkillIds for skill presets
 */
USTRUCT(BlueprintType)
struct FTainlordMasteryDefinition
{
	GENERATED_BODY()

	/** Stable ID for this mastery. Never use array index as identity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery")
	FName MasteryId = NAME_None;

	/** Display name shown in creation UI (e.g. "Berserker", "Arcanist"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery")
	FText DisplayName;

	/** Short summary for UI tooltip or selection card. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery")
	FText ShortDescription;

	/** Archetype derived from this mastery. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery|Derivation")
	ETainlordArchetype DerivedArchetype = ETainlordArchetype::Warrior;

	/** Stat orientation derived from this mastery. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery|Derivation")
	ETainlordStatOrientation DerivedOrientation = ETainlordStatOrientation::Strength;

	/** Primary combat role suggested by this mastery. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery|Derivation")
	ETainlordCombatRole PrimaryCombatRole = ETainlordCombatRole::Frontline;

	/** Recommended elements for this mastery (informational, not enforced). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery|Derivation")
	TArray<ETainlordElement> RecommendedElements;

	/**
	 * Whether this mastery is available in the funding demo.
	 * Filter creation UI to show only demo-approved masteries during early access.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery|Availability")
	bool bIncludedInFundingDemo = true;

	// --- Future extension points (add when needed) ---
	// UPROPERTY(...) TSoftObjectPtr<UTexture2D> IconAsset;
	// UPROPERTY(...) FText FlavorDescription;
	// UPROPERTY(...) TArray<FName> StarterSkillIds;
	// UPROPERTY(...) FTainlordStatBlock BonusStats;

	bool IsValid() const { return !MasteryId.IsNone(); }
};

// ---------------------------------------------------------------------------
// Mastery selection payload
// ---------------------------------------------------------------------------

/**
 * Player's mastery selection during character creation.
 * Stored in FTainlordProfileData.SelectedMasteryId.
 * Runtime systems query the mastery definition catalog to derive gameplay state.
 *
 * Per Arch.md contract:
 * - Profile stores player intent (mastery ID)
 * - Runtime systems derive state (archetype, orientation) from the mastery catalog
 * - No duplicate derivation logic — one source of truth
 */
USTRUCT(BlueprintType)
struct FTainlordMasterySelectionPayload
{
	GENERATED_BODY()

	/** The mastery ID selected during character creation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mastery")
	FName SelectedMasteryId = NAME_None;

	bool HasSelection() const { return !SelectedMasteryId.IsNone(); }
};

// ---------------------------------------------------------------------------
// Mastery catalog placeholder (future: move to UDataAsset or subsystem)
// ---------------------------------------------------------------------------

/**
 * PLACEHOLDER: Future mastery catalog contract.
 * When Stage 3 (Mastery) UI is implemented, replace this with:
 * - A UDataAsset holding TArray<FTainlordMasteryDefinition> MasteryOptions
 * - Or a subsystem that loads mastery definitions from JSON/DataTable
 * - Filtering logic: filter by bIncludedInFundingDemo for demo builds
 *
 * For now, this struct exists to establish the data shape and query contract.
 */
USTRUCT(BlueprintType)
struct FTainlordMasteryCatalogPlaceholder
{
	GENERATED_BODY()

	/** All available mastery options. Populate via DataAsset or subsystem. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mastery|Catalog")
	TArray<FTainlordMasteryDefinition> AvailableMasteries;

	/**
	 * Find a mastery definition by ID.
	 * Returns nullptr if not found.
	 */
	const FTainlordMasteryDefinition* FindMasteryById(FName MasteryId) const
	{
		return AvailableMasteries.FindByPredicate([MasteryId](const FTainlordMasteryDefinition& Def)
		{
			return Def.MasteryId == MasteryId;
		});
	}

	/**
	 * Get all masteries available for the funding demo.
	 * Future: use this to filter the creation UI during early access.
	 */
	TArray<FTainlordMasteryDefinition> GetDemoMasteries() const
	{
		return AvailableMasteries.FilterByPredicate([](const FTainlordMasteryDefinition& Def)
		{
			return Def.bIncludedInFundingDemo;
		});
	}
};
