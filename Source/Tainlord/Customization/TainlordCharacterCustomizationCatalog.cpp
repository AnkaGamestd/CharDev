// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCharacterCustomizationCatalog.h"

// ---------------------------------------------------------------------------
// MatchesFilter: Any-aware filter matching
// ---------------------------------------------------------------------------

bool UTainlordCharacterCustomizationCatalog::MatchesFilter(ECharacterGender EntryFilter, ECharacterGender ContextGender)
{
	// Entry marked Any matches any context gender
	if (EntryFilter == ECharacterGender::Any)
	{
		return true;
	}

	return EntryFilter == ContextGender;
}

bool UTainlordCharacterCustomizationCatalog::MatchesFilter(ECharacterRace EntryFilter, ECharacterRace ContextRace)
{
	// Entry marked Any matches any context race
	if (EntryFilter == ECharacterRace::Any)
	{
		return true;
	}

	return EntryFilter == ContextRace;
}

// ---------------------------------------------------------------------------
// C++ raw pointer lookups (ID only, no context check)
// ---------------------------------------------------------------------------

const FTainlordHeadEntry* UTainlordCharacterCustomizationCatalog::FindHeadEntry(FName HeadId) const
{
	if (HeadId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordHeadEntry& Entry : Heads)
	{
		if (Entry.Id == HeadId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordHairEntry* UTainlordCharacterCustomizationCatalog::FindHairEntry(FName HairId) const
{
	if (HairId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordHairEntry& Entry : Hair)
	{
		if (Entry.Id == HairId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordBeardEntry* UTainlordCharacterCustomizationCatalog::FindBeardEntry(FName BeardId) const
{
	if (BeardId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordBeardEntry& Entry : Beards)
	{
		if (Entry.Id == BeardId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordArmsEntry* UTainlordCharacterCustomizationCatalog::FindArmsEntry(FName ArmsId) const
{
	if (ArmsId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordArmsEntry& Entry : Arms)
	{
		if (Entry.Id == ArmsId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordLegsEntry* UTainlordCharacterCustomizationCatalog::FindLegsEntry(FName LegsId) const
{
	if (LegsId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordLegsEntry& Entry : Legs)
	{
		if (Entry.Id == LegsId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordSkinToneEntry* UTainlordCharacterCustomizationCatalog::FindSkinToneEntry(FName SkinToneId) const
{
	if (SkinToneId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordSkinToneEntry& Entry : SkinTones)
	{
		if (Entry.Id == SkinToneId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

// ---------------------------------------------------------------------------
// Context-aware validation lookups (ID + gender/race gate)
// ---------------------------------------------------------------------------

const FTainlordHeadEntry* UTainlordCharacterCustomizationCatalog::FindHeadEntryForContext(FName HeadId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordHeadEntry* Entry = FindHeadEntry(HeadId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

const FTainlordHairEntry* UTainlordCharacterCustomizationCatalog::FindHairEntryForContext(FName HairId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordHairEntry* Entry = FindHairEntry(HairId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

const FTainlordBeardEntry* UTainlordCharacterCustomizationCatalog::FindBeardEntryForContext(FName BeardId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordBeardEntry* Entry = FindBeardEntry(BeardId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

const FTainlordSkinToneEntry* UTainlordCharacterCustomizationCatalog::FindSkinToneEntryForContext(FName SkinToneId, ECharacterGender /*Gender*/, ECharacterRace /*Race*/) const
{
	// Skin tones are not gender/race gated
	return FindSkinToneEntry(SkinToneId);
}

const FTainlordArmsEntry* UTainlordCharacterCustomizationCatalog::FindArmsEntryForContext(FName ArmsId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordArmsEntry* Entry = FindArmsEntry(ArmsId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

const FTainlordLegsEntry* UTainlordCharacterCustomizationCatalog::FindLegsEntryForContext(FName LegsId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordLegsEntry* Entry = FindLegsEntry(LegsId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

// ---------------------------------------------------------------------------
// Blueprint-safe lookups (by value)
// ---------------------------------------------------------------------------

bool UTainlordCharacterCustomizationCatalog::GetHeadEntry(FName HeadId, FTainlordHeadEntry& OutEntry) const
{
	const FTainlordHeadEntry* Found = FindHeadEntry(HeadId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

bool UTainlordCharacterCustomizationCatalog::GetHairEntry(FName HairId, FTainlordHairEntry& OutEntry) const
{
	const FTainlordHairEntry* Found = FindHairEntry(HairId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

bool UTainlordCharacterCustomizationCatalog::GetBeardEntry(FName BeardId, FTainlordBeardEntry& OutEntry) const
{
	const FTainlordBeardEntry* Found = FindBeardEntry(BeardId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

bool UTainlordCharacterCustomizationCatalog::GetSkinToneEntry(FName SkinToneId, FTainlordSkinToneEntry& OutEntry) const
{
	const FTainlordSkinToneEntry* Found = FindSkinToneEntry(SkinToneId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

bool UTainlordCharacterCustomizationCatalog::GetArmsEntry(FName ArmsId, FTainlordArmsEntry& OutEntry) const
{
	const FTainlordArmsEntry* Found = FindArmsEntry(ArmsId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

bool UTainlordCharacterCustomizationCatalog::GetLegsEntry(FName LegsId, FTainlordLegsEntry& OutEntry) const
{
	const FTainlordLegsEntry* Found = FindLegsEntry(LegsId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Filtered lookups (Any-aware matching)
// ---------------------------------------------------------------------------

TArray<FTainlordHeadEntry> UTainlordCharacterCustomizationCatalog::GetFilteredHeads(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordHeadEntry> Filtered;
	for (const FTainlordHeadEntry& Entry : Heads)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}

TArray<FTainlordHairEntry> UTainlordCharacterCustomizationCatalog::GetFilteredHair(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordHairEntry> Filtered;
	for (const FTainlordHairEntry& Entry : Hair)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}

TArray<FTainlordBeardEntry> UTainlordCharacterCustomizationCatalog::GetFilteredBeards(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordBeardEntry> Filtered;
	for (const FTainlordBeardEntry& Entry : Beards)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}

TArray<FTainlordSkinToneEntry> UTainlordCharacterCustomizationCatalog::GetFilteredSkinTones(ECharacterGender /*Gender*/, ECharacterRace /*Race*/) const
{
	TArray<FTainlordSkinToneEntry> Filtered;
	for (const FTainlordSkinToneEntry& Entry : SkinTones)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		// Skin tones are not gender/race gated
		Filtered.Add(Entry);
	}

	return Filtered;
}

TArray<FTainlordArmsEntry> UTainlordCharacterCustomizationCatalog::GetFilteredArms(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordArmsEntry> Filtered;
	for (const FTainlordArmsEntry& Entry : Arms)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}

TArray<FTainlordLegsEntry> UTainlordCharacterCustomizationCatalog::GetFilteredLegs(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordLegsEntry> Filtered;
	for (const FTainlordLegsEntry& Entry : Legs)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}

// ---------------------------------------------------------------------------
// Accessory lookups — Shoulders
// ---------------------------------------------------------------------------

const FTainlordShouldersEntry* UTainlordCharacterCustomizationCatalog::FindShouldersEntry(FName ShouldersId) const
{
	if (ShouldersId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordShouldersEntry& Entry : Shoulders)
	{
		if (Entry.Id == ShouldersId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordShouldersEntry* UTainlordCharacterCustomizationCatalog::FindShouldersEntryForContext(FName ShouldersId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordShouldersEntry* Entry = FindShouldersEntry(ShouldersId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

// ---------------------------------------------------------------------------
// Accessory lookups — Bracers (Left)
// ---------------------------------------------------------------------------

const FTainlordBracerEntry* UTainlordCharacterCustomizationCatalog::FindBracersLeftEntry(FName LeftBracerId) const
{
	if (LeftBracerId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordBracerEntry& Entry : BracersLeft)
	{
		if (Entry.Id == LeftBracerId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordBracerEntry* UTainlordCharacterCustomizationCatalog::FindBracersLeftEntryForContext(FName LeftBracerId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordBracerEntry* Entry = FindBracersLeftEntry(LeftBracerId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

// ---------------------------------------------------------------------------
// Accessory lookups — Bracers (Right)
// ---------------------------------------------------------------------------

const FTainlordBracerEntry* UTainlordCharacterCustomizationCatalog::FindBracersRightEntry(FName RightBracerId) const
{
	if (RightBracerId.IsNone())
	{
		return nullptr;
	}

	for (const FTainlordBracerEntry& Entry : BracersRight)
	{
		if (Entry.Id == RightBracerId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FTainlordBracerEntry* UTainlordCharacterCustomizationCatalog::FindBracersRightEntryForContext(FName RightBracerId, ECharacterGender Gender, ECharacterRace Race) const
{
	const FTainlordBracerEntry* Entry = FindBracersRightEntry(RightBracerId);
	if (!Entry)
	{
		return nullptr;
	}

	if (!MatchesFilter(Entry->GenderFilter, Gender) || !MatchesFilter(Entry->RaceFilter, Race))
	{
		return nullptr;
	}

	return Entry;
}

// ---------------------------------------------------------------------------
// Blueprint-safe accessory lookups (by value)
// ---------------------------------------------------------------------------

bool UTainlordCharacterCustomizationCatalog::GetShouldersEntry(FName ShouldersId, FTainlordShouldersEntry& OutEntry) const
{
	const FTainlordShouldersEntry* Found = FindShouldersEntry(ShouldersId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

bool UTainlordCharacterCustomizationCatalog::GetBracersLeftEntry(FName LeftBracerId, FTainlordBracerEntry& OutEntry) const
{
	const FTainlordBracerEntry* Found = FindBracersLeftEntry(LeftBracerId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

bool UTainlordCharacterCustomizationCatalog::GetBracersRightEntry(FName RightBracerId, FTainlordBracerEntry& OutEntry) const
{
	const FTainlordBracerEntry* Found = FindBracersRightEntry(RightBracerId);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Filtered accessory lookups (Any-aware matching)
// ---------------------------------------------------------------------------

TArray<FTainlordShouldersEntry> UTainlordCharacterCustomizationCatalog::GetFilteredShoulders(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordShouldersEntry> Filtered;
	for (const FTainlordShouldersEntry& Entry : Shoulders)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}

TArray<FTainlordBracerEntry> UTainlordCharacterCustomizationCatalog::GetFilteredBracersLeft(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordBracerEntry> Filtered;
	for (const FTainlordBracerEntry& Entry : BracersLeft)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}

TArray<FTainlordBracerEntry> UTainlordCharacterCustomizationCatalog::GetFilteredBracersRight(ECharacterGender Gender, ECharacterRace Race) const
{
	TArray<FTainlordBracerEntry> Filtered;
	for (const FTainlordBracerEntry& Entry : BracersRight)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		if (MatchesFilter(Entry.GenderFilter, Gender) && MatchesFilter(Entry.RaceFilter, Race))
		{
			Filtered.Add(Entry);
		}
	}

	return Filtered;
}
