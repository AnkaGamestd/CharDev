// Copyright Kingdawn. All Rights Reserved.
// City domain types — starting city selection and derivation.
// City determines allegiance, initial reputation, spawn point, and narrative context.
// This is a SKELETON CONTRACT for future implementation — no UI, no full catalog yet.

#pragma once

#include "CoreMinimal.h"
#include "TainlordGameplayTypes.h"
#include "TainlordCityTypes.generated.h"

// ---------------------------------------------------------------------------
// City definition
// ---------------------------------------------------------------------------

/**
 * A single city option in the character creation flow.
 * Each city represents a distinct starting location with narrative and gameplay implications.
 *
 * Per product design:
 * - Player picks ONE starting city during character creation (Stage 4)
 * - City derives allegiance, initial reputation, spawn point
 * - This struct is the stable-ID catalog entry contract
 *
 * Future extension points:
 * - Add TSoftObjectPtr<UTexture2D> MapPreviewAsset for UI preview
 * - Add FText LoreDescription for narrative context
 * - Add FVector SpawnWorldLocation for runtime spawn
 * - Add TMap<ETainlordReputationAxis, int32> InitialReputationBonuses
 */
USTRUCT(BlueprintType)
struct FTainlordCityDefinition
{
	GENERATED_BODY()

	/** Stable ID for this city. Never use array index as identity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City")
	FName CityId = NAME_None;

	/** Display name shown in creation UI (e.g. "Ironhold", "Silverreach"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City")
	FText DisplayName;

	/** Short summary for UI tooltip or selection card. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City")
	FText ShortDescription;

	/**
	 * Whether this city is recommended for new players.
	 * UI can highlight recommended cities in the selection screen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Availability")
	bool bRecommendedForNewPlayers = false;

	/** Allegiance derived from this city. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Derivation")
	ETainlordAllegiance DerivedAllegiance = ETainlordAllegiance::City;

	/**
	 * Starting reputation bonus granted by this city.
	 * Applied to CityReputation on character creation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Derivation")
	int32 StartingCityReputation = 0;

	/**
	 * Starting mercenary reputation bonus granted by this city.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Derivation")
	int32 StartingMercenaryReputation = 0;

	/**
	 * Map/level name to load when spawning into this city.
	 * Used by the spawn system after character creation completes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Spawn")
	FName StartingMapName = NAME_None;

	/**
	 * Whether this city is available in the funding demo.
	 * Filter creation UI to show only demo-approved cities during early access.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Availability")
	bool bIncludedInFundingDemo = true;

	// --- Future extension points (add when needed) ---
	// UPROPERTY(...) TSoftObjectPtr<UTexture2D> MapPreviewAsset;
	// UPROPERTY(...) FText LoreDescription;
	// UPROPERTY(...) FVector SpawnWorldLocation;
	// UPROPERTY(...) TMap<ETainlordReputationAxis, int32> InitialReputationBonuses;
	// UPROPERTY(...) TArray<FName> AvailableQuestChains;

	bool IsValid() const { return !CityId.IsNone(); }
};

// ---------------------------------------------------------------------------
// City selection payload
// ---------------------------------------------------------------------------

/**
 * Player's city selection during character creation.
 * Stored in FTainlordProfileData.SelectedCityId.
 * Runtime systems query the city definition catalog to derive spawn and reputation state.
 *
 * Per Arch.md contract:
 * - Profile stores player intent (city ID)
 * - Runtime systems derive state (allegiance, reputation, spawn) from the city catalog
 * - No duplicate derivation logic — one source of truth
 */
USTRUCT(BlueprintType)
struct FTainlordCitySelectionPayload
{
	GENERATED_BODY()

	/** The city ID selected during character creation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
	FName SelectedCityId = NAME_None;

	bool HasSelection() const { return !SelectedCityId.IsNone(); }
};

// ---------------------------------------------------------------------------
// City catalog placeholder (future: move to UDataAsset or subsystem)
// ---------------------------------------------------------------------------

/**
 * PLACEHOLDER: Future city catalog contract.
 * When Stage 4 (City) UI is implemented, replace this with:
 * - A UDataAsset holding TArray<FTainlordCityDefinition> CityOptions
 * - Or a subsystem that loads city definitions from JSON/DataTable
 * - Filtering logic: filter by bIncludedInFundingDemo for demo builds
 *
 * For now, this struct exists to establish the data shape and query contract.
 */
USTRUCT(BlueprintType)
struct FTainlordCityCatalogPlaceholder
{
	GENERATED_BODY()

	/** All available city options. Populate via DataAsset or subsystem. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Catalog")
	TArray<FTainlordCityDefinition> AvailableCities;

	/**
	 * Find a city definition by ID.
	 * Returns nullptr if not found.
	 */
	const FTainlordCityDefinition* FindCityById(FName CityId) const
	{
		return AvailableCities.FindByPredicate([CityId](const FTainlordCityDefinition& Def)
		{
			return Def.CityId == CityId;
		});
	}

	/**
	 * Get all cities available for the funding demo.
	 * Future: use this to filter the creation UI during early access.
	 */
	TArray<FTainlordCityDefinition> GetDemoCities() const
	{
		return AvailableCities.FilterByPredicate([](const FTainlordCityDefinition& Def)
		{
			return Def.bIncludedInFundingDemo;
		});
	}
};
