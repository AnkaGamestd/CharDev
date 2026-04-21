// Copyright Kingdawn. All Rights Reserved.
// Catalog and filtering registry for character customization options.
// Per Arch.md: This is a lookup registry only, does not store player selection.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCharacterCustomizationCatalog.generated.h"

/**
 * Lookup and filtering registry for character customization options.
 * Contains authorable option arrays for heads, hair, beards, skin tones.
 * Per Arch.md: This catalog does not store active player selection.
 */
UCLASS(BlueprintType)
class TAINLORD_API UTainlordCharacterCustomizationCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All available head options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog")
	TArray<FTainlordHeadEntry> Heads;

	/** All available hair options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog")
	TArray<FTainlordHairEntry> Hair;

	/** All available beard options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog")
	TArray<FTainlordBeardEntry> Beards;

	/** All available arms options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog")
	TArray<FTainlordArmsEntry> Arms;

	/** All available legs options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog")
	TArray<FTainlordLegsEntry> Legs;

	/** All available skin tone options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog")
	TArray<FTainlordSkinToneEntry> SkinTones;

	/** All available shoulders accessory options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog|Accessories")
	TArray<FTainlordShouldersEntry> Shoulders;

	/** All available left bracer accessory options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog|Accessories")
	TArray<FTainlordBracerEntry> BracersLeft;

	/** All available right bracer accessory options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog|Accessories")
	TArray<FTainlordBracerEntry> BracersRight;

	// --- Lookup helpers (C++ only, return raw pointer) ---

	/** Find a head entry by stable ID. Returns null if not found. */
	const FTainlordHeadEntry* FindHeadEntry(FName HeadId) const;

	/** Find a hair entry by stable ID. Returns null if not found. */
	const FTainlordHairEntry* FindHairEntry(FName HairId) const;

	/** Find a beard entry by stable ID. Returns null if not found. */
	const FTainlordBeardEntry* FindBeardEntry(FName BeardId) const;

	/** Find a skin tone entry by stable ID. Returns null if not found. */
	const FTainlordSkinToneEntry* FindSkinToneEntry(FName SkinToneId) const;

	/** Find an arms entry by stable ID. Returns null if not found. */
	const FTainlordArmsEntry* FindArmsEntry(FName ArmsId) const;

	/** Find a legs entry by stable ID. Returns null if not found. */
	const FTainlordLegsEntry* FindLegsEntry(FName LegsId) const;

	/** Find a shoulders entry by stable ID. Returns null if not found. */
	const FTainlordShouldersEntry* FindShouldersEntry(FName ShouldersId) const;

	/** Find a bracer entry by stable ID (unified — searches BracersLeft). Returns null if not found. */
	const FTainlordBracerEntry* FindBracerEntry(FName BracerId) const;

	// Deprecated: use FindBracerEntry
	const FTainlordBracerEntry* FindBracersLeftEntry(FName LeftBracerId) const;
	const FTainlordBracerEntry* FindBracersRightEntry(FName RightBracerId) const;

	// --- Context-aware validation lookups (C++ only) ---

	/**
	 * Find a head entry by ID and validate it is allowed for the given gender/race context.
	 * Returns null if not found or if the entry's filters exclude this context.
	 */
	const FTainlordHeadEntry* FindHeadEntryForContext(FName HeadId, ECharacterGender Gender, ECharacterRace Race) const;

	/** Find a hair entry by ID and validate context. Returns null if not allowed. */
	const FTainlordHairEntry* FindHairEntryForContext(FName HairId, ECharacterGender Gender, ECharacterRace Race) const;

	/** Find a beard entry by ID and validate context. Returns null if not allowed. */
	const FTainlordBeardEntry* FindBeardEntryForContext(FName BeardId, ECharacterGender Gender, ECharacterRace Race) const;

	/** Find a skin tone entry by ID. Skin tones are not gender/race gated. */
	const FTainlordSkinToneEntry* FindSkinToneEntryForContext(FName SkinToneId, ECharacterGender Gender, ECharacterRace Race) const;

	/** Find an arms entry by ID and validate context. Returns null if not allowed. */
	const FTainlordArmsEntry* FindArmsEntryForContext(FName ArmsId, ECharacterGender Gender, ECharacterRace Race) const;

	/** Find a legs entry by ID and validate context. Returns null if not allowed. */
	const FTainlordLegsEntry* FindLegsEntryForContext(FName LegsId, ECharacterGender Gender, ECharacterRace Race) const;

	/** Find a shoulders entry by ID and validate context. Returns null if not allowed. */
	const FTainlordShouldersEntry* FindShouldersEntryForContext(FName ShouldersId, ECharacterGender Gender, ECharacterRace Race) const;

	/** Find a bracer entry by ID and validate context (unified — searches BracersLeft). Returns null if not allowed. */
	const FTainlordBracerEntry* FindBracerEntryForContext(FName BracerId, ECharacterGender Gender, ECharacterRace Race) const;

	// Deprecated: use FindBracerEntryForContext
	const FTainlordBracerEntry* FindBracersLeftEntryForContext(FName LeftBracerId, ECharacterGender Gender, ECharacterRace Race) const;
	const FTainlordBracerEntry* FindBracersRightEntryForContext(FName RightBracerId, ECharacterGender Gender, ECharacterRace Race) const;

	// --- Blueprint-safe lookup helpers (return by value with bool success) ---

	/** Get a head entry by stable ID. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetHeadEntry(FName HeadId, FTainlordHeadEntry& OutEntry) const;

	/** Get a hair entry by stable ID. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetHairEntry(FName HairId, FTainlordHairEntry& OutEntry) const;

	/** Get a beard entry by stable ID. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetBeardEntry(FName BeardId, FTainlordBeardEntry& OutEntry) const;

	/** Get a skin tone entry by stable ID. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetSkinToneEntry(FName SkinToneId, FTainlordSkinToneEntry& OutEntry) const;

	/** Get an arms entry by stable ID. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetArmsEntry(FName ArmsId, FTainlordArmsEntry& OutEntry) const;

	/** Get a legs entry by stable ID. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetLegsEntry(FName LegsId, FTainlordLegsEntry& OutEntry) const;

	/** Get a shoulders entry by stable ID. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetShouldersEntry(FName ShouldersId, FTainlordShouldersEntry& OutEntry) const;

	/** Get a bracer entry by stable ID (unified). Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	bool GetBracerEntry(FName BracerId, FTainlordBracerEntry& OutEntry) const;

	// --- Filtered lookup helpers ---

	/** Get all head entries allowed for the given gender and race. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordHeadEntry> GetFilteredHeads(ECharacterGender Gender, ECharacterRace Race) const;

	/** Get all hair entries allowed for the given gender and race. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordHairEntry> GetFilteredHair(ECharacterGender Gender, ECharacterRace Race) const;

	/** Get all beard entries allowed for the given gender and race. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordBeardEntry> GetFilteredBeards(ECharacterGender Gender, ECharacterRace Race) const;

	/** Get all skin tone entries (skin tones are not gender/race filtered). */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordSkinToneEntry> GetFilteredSkinTones(ECharacterGender Gender, ECharacterRace Race) const;

	/** Get all arms entries allowed for the given gender and race. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordArmsEntry> GetFilteredArms(ECharacterGender Gender, ECharacterRace Race) const;

	/** Get all legs entries allowed for the given gender and race. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordLegsEntry> GetFilteredLegs(ECharacterGender Gender, ECharacterRace Race) const;

	/** Get all shoulders entries allowed for the given gender and race. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordShouldersEntry> GetFilteredShoulders(ECharacterGender Gender, ECharacterRace Race) const;

	/** Get all bracer entries allowed for the given gender and race (unified — from BracersLeft). */
	UFUNCTION(BlueprintCallable, Category = "Customization|Catalog")
	TArray<FTainlordBracerEntry> GetFilteredBracers(ECharacterGender Gender, ECharacterRace Race) const;

private:
	/**
	 * Check if an entry's filter matches the given context.
	 * An entry with filter=Any matches any context.
	 * An entry with a specific filter matches only that specific value.
	 */
	static bool MatchesFilter(ECharacterGender EntryFilter, ECharacterGender ContextGender);
	static bool MatchesFilter(ECharacterRace EntryFilter, ECharacterRace ContextRace);
};
