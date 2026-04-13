// Copyright Kingdawn. All Rights Reserved.

#include "TainlordDemoCatalogLibrary.h"

// ===========================================================================
// MASTERY CATALOG
// ===========================================================================

const TArray<FTainlordMasteryDefinition>& UTainlordDemoCatalogLibrary::GetCachedDemoMasteries()
{
	static TArray<FTainlordMasteryDefinition> CachedMasteries;
	
	if (CachedMasteries.Num() == 0)
	{
		// Build the demo mastery catalog
		// These are stable-ID entries with full derivation rules
		
		{
			FTainlordMasteryDefinition Mastery;
			Mastery.MasteryId = FName(TEXT("Berserker"));
			Mastery.DisplayName = FText::FromString(TEXT("Berserker"));
			Mastery.ShortDescription = FText::FromString(TEXT("Fury-driven warrior. High damage, low defense."));
			Mastery.DerivedArchetype = ETainlordArchetype::Warrior;
			Mastery.DerivedOrientation = ETainlordStatOrientation::Strength;
			Mastery.PrimaryCombatRole = ETainlordCombatRole::Frontline;
			Mastery.RecommendedElements = { ETainlordElement::Fire };
			Mastery.bIncludedInFundingDemo = true;
			CachedMasteries.Add(Mastery);
		}
		
		{
			FTainlordMasteryDefinition Mastery;
			Mastery.MasteryId = FName(TEXT("Guardian"));
			Mastery.DisplayName = FText::FromString(TEXT("Guardian"));
			Mastery.ShortDescription = FText::FromString(TEXT("Defensive tank. High vitality, protects allies."));
			Mastery.DerivedArchetype = ETainlordArchetype::Warrior;
			Mastery.DerivedOrientation = ETainlordStatOrientation::Strength;
			Mastery.PrimaryCombatRole = ETainlordCombatRole::Frontline;
			Mastery.RecommendedElements = { ETainlordElement::Earth };
			Mastery.bIncludedInFundingDemo = true;
			CachedMasteries.Add(Mastery);
		}
		
		{
			FTainlordMasteryDefinition Mastery;
			Mastery.MasteryId = FName(TEXT("Arcanist"));
			Mastery.DisplayName = FText::FromString(TEXT("Arcanist"));
			Mastery.ShortDescription = FText::FromString(TEXT("Arcane scholar. High spell power, elemental mastery."));
			Mastery.DerivedArchetype = ETainlordArchetype::Mage;
			Mastery.DerivedOrientation = ETainlordStatOrientation::Intelligence;
			Mastery.PrimaryCombatRole = ETainlordCombatRole::RangedDamage;
			Mastery.RecommendedElements = { ETainlordElement::Fire, ETainlordElement::Water };
			Mastery.bIncludedInFundingDemo = true;
			CachedMasteries.Add(Mastery);
		}
		
		{
			FTainlordMasteryDefinition Mastery;
			Mastery.MasteryId = FName(TEXT("Cleric"));
			Mastery.DisplayName = FText::FromString(TEXT("Cleric"));
			Mastery.ShortDescription = FText::FromString(TEXT("Divine healer. Sustains allies, removes afflictions."));
			Mastery.DerivedArchetype = ETainlordArchetype::Cleric;
			Mastery.DerivedOrientation = ETainlordStatOrientation::Intelligence;
			Mastery.PrimaryCombatRole = ETainlordCombatRole::SustainSupport;
			Mastery.RecommendedElements = { ETainlordElement::Water };
			Mastery.bIncludedInFundingDemo = true;
			CachedMasteries.Add(Mastery);
		}
		
		{
			FTainlordMasteryDefinition Mastery;
			Mastery.MasteryId = FName(TEXT("Ranger"));
			Mastery.DisplayName = FText::FromString(TEXT("Ranger"));
			Mastery.ShortDescription = FText::FromString(TEXT("Precision marksman. High range, mobility."));
			Mastery.DerivedArchetype = ETainlordArchetype::Archer;
			Mastery.DerivedOrientation = ETainlordStatOrientation::Strength;
			Mastery.PrimaryCombatRole = ETainlordCombatRole::RangedDamage;
			Mastery.RecommendedElements = { ETainlordElement::Wind };
			Mastery.bIncludedInFundingDemo = true;
			CachedMasteries.Add(Mastery);
		}
		
		{
			FTainlordMasteryDefinition Mastery;
			Mastery.MasteryId = FName(TEXT("Shadowblade"));
			Mastery.DisplayName = FText::FromString(TEXT("Shadowblade"));
			Mastery.ShortDescription = FText::FromString(TEXT("Agile assassin. High burst, tempo control."));
			Mastery.DerivedArchetype = ETainlordArchetype::Dagger;
			Mastery.DerivedOrientation = ETainlordStatOrientation::Hybrid;
			Mastery.PrimaryCombatRole = ETainlordCombatRole::TempoSupport;
			Mastery.RecommendedElements = { ETainlordElement::Wind, ETainlordElement::Fire };
			Mastery.bIncludedInFundingDemo = true;
			CachedMasteries.Add(Mastery);
		}
		
		UE_LOG(LogTemp, Log, TEXT("DemoCatalog: Initialized %d demo masteries"), CachedMasteries.Num());
	}
	
	return CachedMasteries;
}

TArray<FTainlordMasteryDefinition> UTainlordDemoCatalogLibrary::GetDemoMasteries()
{
	return GetCachedDemoMasteries();
}

FTainlordMasteryDefinition UTainlordDemoCatalogLibrary::FindMasteryById(FName MasteryId)
{
	const TArray<FTainlordMasteryDefinition>& Masteries = GetCachedDemoMasteries();
	
	const FTainlordMasteryDefinition* Found = Masteries.FindByPredicate([MasteryId](const FTainlordMasteryDefinition& Def)
	{
		return Def.MasteryId == MasteryId;
	});
	
	if (Found)
	{
		return *Found;
	}
	
	// Return invalid entry
	return FTainlordMasteryDefinition();
}

bool UTainlordDemoCatalogLibrary::IsValidMasteryId(FName MasteryId)
{
	FTainlordMasteryDefinition Found = FindMasteryById(MasteryId);
	return Found.IsValid();
}

int32 UTainlordDemoCatalogLibrary::GetDemoMasteryCount()
{
	return GetCachedDemoMasteries().Num();
}

// ===========================================================================
// CITY CATALOG
// ===========================================================================

const TArray<FTainlordCityDefinition>& UTainlordDemoCatalogLibrary::GetCachedDemoCities()
{
	static TArray<FTainlordCityDefinition> CachedCities;
	
	if (CachedCities.Num() == 0)
	{
		// Build the demo city catalog
		// These are stable-ID entries with full derivation rules
		
		{
			FTainlordCityDefinition City;
			City.CityId = FName(TEXT("Ironhold"));
			City.DisplayName = FText::FromString(TEXT("Ironhold"));
			City.ShortDescription = FText::FromString(TEXT("Fortified capital. High city reputation, safe haven."));
			City.bRecommendedForNewPlayers = true;
			City.DerivedAllegiance = ETainlordAllegiance::City;
			City.StartingCityReputation = 10;
			City.StartingMercenaryReputation = 0;
			City.StartingMapName = FName(TEXT("Map_Ironhold"));
			City.bIncludedInFundingDemo = true;
			CachedCities.Add(City);
		}
		
		{
			FTainlordCityDefinition City;
			City.CityId = FName(TEXT("Silverreach"));
			City.DisplayName = FText::FromString(TEXT("Silverreach"));
			City.ShortDescription = FText::FromString(TEXT("Trade hub. Balanced reputation, mercenary opportunities."));
			City.bRecommendedForNewPlayers = true;
			City.DerivedAllegiance = ETainlordAllegiance::City;
			City.StartingCityReputation = 5;
			City.StartingMercenaryReputation = 5;
			City.StartingMapName = FName(TEXT("Map_Silverreach"));
			City.bIncludedInFundingDemo = true;
			CachedCities.Add(City);
		}
		
		{
			FTainlordCityDefinition City;
			City.CityId = FName(TEXT("Thornwall"));
			City.DisplayName = FText::FromString(TEXT("Thornwall"));
			City.ShortDescription = FText::FromString(TEXT("Frontier outpost. High mercenary reputation, lawless."));
			City.bRecommendedForNewPlayers = false;
			City.DerivedAllegiance = ETainlordAllegiance::City;
			City.StartingCityReputation = 0;
			City.StartingMercenaryReputation = 10;
			City.StartingMapName = FName(TEXT("Map_Thornwall"));
			City.bIncludedInFundingDemo = true;
			CachedCities.Add(City);
		}
		
		{
			FTainlordCityDefinition City;
			City.CityId = FName(TEXT("Ashfall"));
			City.DisplayName = FText::FromString(TEXT("Ashfall"));
			City.ShortDescription = FText::FromString(TEXT("Exiled stronghold. High infamy, outlaw path."));
			City.bRecommendedForNewPlayers = false;
			City.DerivedAllegiance = ETainlordAllegiance::Exile;
			City.StartingCityReputation = -10;
			City.StartingMercenaryReputation = 5;
			City.StartingMapName = FName(TEXT("Map_Ashfall"));
			City.bIncludedInFundingDemo = true;
			CachedCities.Add(City);
		}
		
		UE_LOG(LogTemp, Log, TEXT("DemoCatalog: Initialized %d demo cities"), CachedCities.Num());
	}
	
	return CachedCities;
}

TArray<FTainlordCityDefinition> UTainlordDemoCatalogLibrary::GetDemoCities()
{
	return GetCachedDemoCities();
}

FTainlordCityDefinition UTainlordDemoCatalogLibrary::FindCityById(FName CityId)
{
	const TArray<FTainlordCityDefinition>& Cities = GetCachedDemoCities();
	
	const FTainlordCityDefinition* Found = Cities.FindByPredicate([CityId](const FTainlordCityDefinition& Def)
	{
		return Def.CityId == CityId;
	});
	
	if (Found)
	{
		return *Found;
	}
	
	// Return invalid entry
	return FTainlordCityDefinition();
}

bool UTainlordDemoCatalogLibrary::IsValidCityId(FName CityId)
{
	FTainlordCityDefinition Found = FindCityById(CityId);
	return Found.IsValid();
}

int32 UTainlordDemoCatalogLibrary::GetDemoCityCount()
{
	return GetCachedDemoCities().Num();
}
