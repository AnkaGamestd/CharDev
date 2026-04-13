// Copyright Kingdawn. All Rights Reserved.
// Demo catalog library — provides placeholder mastery and city data for the creation flow.
// All entries use stable FName IDs. No index-based access.
// When production data assets are ready, replace the hardcoded data with DataAsset loading.
// This library is the UI's data source for Mastery (Stage 3) and City (Stage 4) screens.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TainlordMasteryTypes.h"
#include "TainlordCityTypes.h"
#include "TainlordDemoCatalogLibrary.generated.h"

/**
 * Static function library providing demo mastery and city catalog data.
 *
 * UI screens call these functions to populate their selection grids:
 *   Mastery Screen → GetDemoMasteries() → populate card grid
 *   City Screen    → GetDemoCities()    → populate card grid
 *
 * Lookup by stable ID:
 *   FindMasteryById("Berserker") → FTainlordMasteryDefinition
 *   FindCityById("Ironhold")     → FTainlordCityDefinition
 *
 * When production catalogs are ready:
 *   1. Create UDataAsset subclasses for mastery and city catalogs
 *   2. Replace this library's internals to load from DataAsset
 *   3. Keep the same function signatures — UI code doesn't change
 */
UCLASS()
class TAINLORD_API UTainlordDemoCatalogLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// -----------------------------------------------------------------------
	// Mastery catalog
	// -----------------------------------------------------------------------

	/**
	 * Get all mastery definitions available in the demo build.
	 * UI populates its mastery selection grid from this list.
	 *
	 * Demo masteries (stable IDs):
	 *   "Berserker"  — Warrior / Strength / Frontline
	 *   "Guardian"   — Warrior / Vitality / Frontline
	 *   "Arcanist"   — Mage / Intelligence / RangedDamage
	 *   "Cleric"     — Cleric / Willpower / SustainSupport
	 *   "Ranger"     — Archer / Strength / RangedDamage
	 *   "Shadowblade"— Dagger / Hybrid / TempoSupport
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|Mastery")
	static TArray<FTainlordMasteryDefinition> GetDemoMasteries();

	/**
	 * Find a mastery definition by its stable ID.
	 * Returns an invalid entry (IsValid() == false) if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|Mastery")
	static FTainlordMasteryDefinition FindMasteryById(FName MasteryId);

	/**
	 * Check if a mastery ID exists in the demo catalog.
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|Mastery")
	static bool IsValidMasteryId(FName MasteryId);

	/**
	 * Get the total number of demo masteries.
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|Mastery")
	static int32 GetDemoMasteryCount();

	// -----------------------------------------------------------------------
	// City catalog
	// -----------------------------------------------------------------------

	/**
	 * Get all city definitions available in the demo build.
	 * UI populates its city selection grid from this list.
	 *
	 * Demo cities (stable IDs):
	 *   "Ironhold"    — City allegiance, +10 CityRep, map: "Map_Ironhold"
	 *   "Silverreach" — City allegiance, +5 CityRep, +5 MercRep, map: "Map_Silverreach"
	 *   "Thornwall"   — City allegiance, +10 MercRep, map: "Map_Thornwall"
	 *   "Ashfall"     — Exile allegiance, +5 Infamy, map: "Map_Ashfall"
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|City")
	static TArray<FTainlordCityDefinition> GetDemoCities();

	/**
	 * Find a city definition by its stable ID.
	 * Returns an invalid entry (IsValid() == false) if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|City")
	static FTainlordCityDefinition FindCityById(FName CityId);

	/**
	 * Check if a city ID exists in the demo catalog.
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|City")
	static bool IsValidCityId(FName CityId);

	/**
	 * Get the total number of demo cities.
	 */
	UFUNCTION(BlueprintPure, Category = "Demo|Catalog|City")
	static int32 GetDemoCityCount();

private:

	/** Build and cache the demo mastery list. Called once, then cached. */
	static const TArray<FTainlordMasteryDefinition>& GetCachedDemoMasteries();

	/** Build and cache the demo city list. Called once, then cached. */
	static const TArray<FTainlordCityDefinition>& GetCachedDemoCities();
};
