// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCharacterCreationLibrary.h"
#include "Customization/TainlordCharacterCustomizationCatalog.h"
#include "Customization/TainlordCharacterAppearanceComponent.h"
#include "Customization/TainlordCharacterProfileSubsystem.h"
#include "Customization/TainlordCharacterSpawnLibrary.h"
#include "Engine/GameInstance.h"

// ---------------------------------------------------------------------------
// FindCatalog — resolve the catalog from the subsystem or world
// ---------------------------------------------------------------------------

UTainlordCharacterCustomizationCatalog* UTainlordCharacterCreationLibrary::FindCatalog(const UObject* WorldContext)
{
	// CRITICAL FIX: Load the canonical catalog directly from the known path.
	// This prevents loading stale catalogs from other locations (e.g., KingdawnCombat/Data).
	// The Tainlord customization catalog is always at /Game/Tainlord/Data/DA_CharacterCustomizationCatalog.
	static const TCHAR* CanonicalCatalogPath = TEXT("/Game/Tainlord/Data/DA_CharacterCustomizationCatalog");
	
	UTainlordCharacterCustomizationCatalog* Catalog = LoadObject<UTainlordCharacterCustomizationCatalog>(
		nullptr,
		CanonicalCatalogPath,
		nullptr,
		LOAD_None,
		nullptr
	);
	
	if (!Catalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: FindCatalog - failed to load catalog from '%s'"),
			CanonicalCatalogPath);
		return nullptr;
	}
	
	UE_LOG(LogTemp, Log, TEXT("TainlordCreation: FindCatalog - loaded catalog from '%s'"), CanonicalCatalogPath);
	return Catalog;
}

// ---------------------------------------------------------------------------
// Filtering queries
// ---------------------------------------------------------------------------

TArray<FTainlordHeadEntry> UTainlordCharacterCreationLibrary::GetAvailableHeads(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race)
{
	UTainlordCharacterCustomizationCatalog* Catalog = FindCatalog(WorldContext);
	if (!Catalog)
	{
		return TArray<FTainlordHeadEntry>();
	}

	return Catalog->GetFilteredHeads(Gender, Race);
}

TArray<FTainlordHairEntry> UTainlordCharacterCreationLibrary::GetAvailableHair(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race)
{
	UTainlordCharacterCustomizationCatalog* Catalog = FindCatalog(WorldContext);
	if (!Catalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: GetAvailableHair - catalog is null"));
		return TArray<FTainlordHairEntry>();
	}

	TArray<FTainlordHairEntry> FilteredHair = Catalog->GetFilteredHair(Gender, Race);
	UE_LOG(LogTemp, Log, TEXT("TainlordCreation: GetAvailableHair - Gender=%d, Race=%d, Found=%d entries"),
		static_cast<int32>(Gender), static_cast<int32>(Race), FilteredHair.Num());
	
	for (const FTainlordHairEntry& Entry : FilteredHair)
	{
		UE_LOG(LogTemp, Log, TEXT("  - Hair entry: Id=%s, MeshAsset=%s"),
			*Entry.Id.ToString(), *Entry.MeshAsset.ToString());
	}
	
	return FilteredHair;
}

TArray<FTainlordBeardEntry> UTainlordCharacterCreationLibrary::GetAvailableBeards(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race)
{
	UTainlordCharacterCustomizationCatalog* Catalog = FindCatalog(WorldContext);
	if (!Catalog)
	{
		return TArray<FTainlordBeardEntry>();
	}

	return Catalog->GetFilteredBeards(Gender, Race);
}

TArray<FTainlordSkinToneEntry> UTainlordCharacterCreationLibrary::GetAvailableSkinTones(const UObject* WorldContext)
{
	UTainlordCharacterCustomizationCatalog* Catalog = FindCatalog(WorldContext);
	if (!Catalog)
	{
		return TArray<FTainlordSkinToneEntry>();
	}

	// Skin tones are not gender/race gated — pass Any to get all
	return Catalog->GetFilteredSkinTones(ECharacterGender::Any, ECharacterRace::Any);
}

// ---------------------------------------------------------------------------
// Live preview — delegates to SpawnLibrary (shared apply path, no duplication)
// ---------------------------------------------------------------------------

bool UTainlordCharacterCreationLibrary::ApplyPreview(AActor* PreviewCharacter, const FTainlordProfileData& WorkingProfile)
{
	if (!PreviewCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: ApplyPreview - null PreviewCharacter"));
		return false;
	}

	// Use the SAME authoritative apply point as spawn — per Arch.md shared apply path.
	return UTainlordCharacterSpawnLibrary::ApplyProfileToCharacter(PreviewCharacter, WorkingProfile);
}

bool UTainlordCharacterCreationLibrary::ApplyPreviewSlot(AActor* PreviewCharacter, const FTainlordProfileData& WorkingProfile, FName SlotName, FName NewId)
{
	if (!PreviewCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: ApplyPreviewSlot - null PreviewCharacter"));
		return false;
	}

	UTainlordCharacterAppearanceComponent* AppearanceComp = UTainlordCharacterSpawnLibrary::GetAppearanceComponent(PreviewCharacter);
	if (!AppearanceComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: ApplyPreviewSlot - no AppearanceComponent on '%s'"), *PreviewCharacter->GetName());
		return false;
	}

	// Ensure profile context is set (gender/race from working profile)
	AppearanceComp->SetProfileContext(WorkingProfile.Gender, WorkingProfile.Race);

	// Apply the individual slot
	const FString SlotStr = SlotName.ToString();

	if (SlotStr == TEXT("Head"))
	{
		return AppearanceComp->ApplyHead(NewId);
	}
	else if (SlotStr == TEXT("Hair"))
	{
		return AppearanceComp->ApplyHair(NewId);
	}
	else if (SlotStr == TEXT("Beard"))
	{
		return AppearanceComp->ApplyBeard(NewId);
	}
	else if (SlotStr == TEXT("SkinTone"))
	{
		return AppearanceComp->ApplySkinTone(NewId);
	}
	else if (SlotStr == TEXT("Shoulders"))
	{
		return AppearanceComp->ApplyShoulders(NewId);
	}
	else if (SlotStr == TEXT("LeftBracer"))
	{
		return AppearanceComp->ApplyLeftBracer(NewId);
	}
	else if (SlotStr == TEXT("RightBracer"))
	{
		return AppearanceComp->ApplyRightBracer(NewId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: ApplyPreviewSlot - unknown slot name '%s'. Valid: Head, Hair, Beard, SkinTone, Shoulders, LeftBracer, RightBracer"), *SlotStr);
		return false;
	}
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool UTainlordCharacterCreationLibrary::ValidateProfile(const FTainlordProfileData& Profile, FString& OutReason)
{
	// Legacy full-validation entry point — delegates to staged validation
	FProfileValidationResult Result = ValidateProfileForStage(Profile, ECreationStage::Confirm);
	OutReason = Result.Reason;
	return Result.bIsValid;
}

FProfileValidationResult UTainlordCharacterCreationLibrary::ValidateForConfirm(const FTainlordProfileData& Profile)
{
	return ValidateProfileForStage(Profile, ECreationStage::Confirm);
}

FProfileValidationResult UTainlordCharacterCreationLibrary::ValidateProfileForStage(const FTainlordProfileData& Profile, ECreationStage Stage)
{
	// Validation is cumulative — we check all requirements up to and including the current stage.

	// --- Stage 1: RaceSelect ---
	// Required: Gender != Any, Race != Any
	if (Stage >= ECreationStage::RaceSelect)
	{
		if (Profile.Gender == ECharacterGender::Any)
		{
			return FProfileValidationResult::Fail(ECreationStage::RaceSelect,
				TEXT("A specific gender must be selected (cannot be 'Any')."));
		}

		if (Profile.Race == ECharacterRace::Any)
		{
			return FProfileValidationResult::Fail(ECreationStage::RaceSelect,
				TEXT("A specific race must be selected (cannot be 'Any')."));
		}
	}

	// --- Stage 2: Appearance ---
	// No mandatory checks — appearance is optional (head/hair/beard can be None).
	// If the UI wants to enforce a minimum (e.g., must pick at least a head),
	// that can be added here. For now, we allow players to skip customization.

	// --- Stage 3: Mastery ---
	// Required: SelectedMasteryId != None
	if (Stage >= ECreationStage::Mastery)
	{
		if (Profile.SelectedMasteryId.IsNone())
		{
			return FProfileValidationResult::Fail(ECreationStage::Mastery,
				TEXT("A mastery (class specialization) must be selected."));
		}

		// Future: if mastery catalog exists, validate that SelectedMasteryId is a valid entry.
		// For now, we trust that the UI only allows valid selections.
	}

	// --- Stage 4: City ---
	// Required: SelectedCityId != None
	if (Stage >= ECreationStage::City)
	{
		if (Profile.SelectedCityId.IsNone())
		{
			return FProfileValidationResult::Fail(ECreationStage::City,
				TEXT("A starting city must be selected."));
		}

		// Future: if city catalog exists, validate that SelectedCityId is a valid entry.
	}

	// --- Stage 5: Name ---
	// Required: CharacterName is not empty, 2-32 chars
	if (Stage >= ECreationStage::Name)
	{
		if (Profile.CharacterName.IsEmpty())
		{
			return FProfileValidationResult::Fail(ECreationStage::Name,
				TEXT("Character name is required."));
		}

		if (Profile.CharacterName.Len() < 2)
		{
			return FProfileValidationResult::Fail(ECreationStage::Name,
				TEXT("Character name must be at least 2 characters."));
		}

		if (Profile.CharacterName.Len() > 32)
		{
			return FProfileValidationResult::Fail(ECreationStage::Name,
				TEXT("Character name must not exceed 32 characters."));
		}

		// Future: name profanity filter, uniqueness check (if server-side validation is added)
	}

	// --- Stage 6: Confirm ---
	// All previous validations must pass (already checked above).
	// No additional requirements beyond cumulative checks.

	return FProfileValidationResult::Success();
}

// ---------------------------------------------------------------------------
// Context-aware re-filter (sanitize after gender/race change)
// ---------------------------------------------------------------------------

bool UTainlordCharacterCreationLibrary::SanitizeAppearanceForContext(const UObject* WorldContext, FTainlordProfileData& InOutProfile)
{
	UTainlordCharacterCustomizationCatalog* Catalog = FindCatalog(WorldContext);
	if (!Catalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: SanitizeAppearanceForContext - no catalog found, cannot validate"));
		return false;
	}

	bool bAnyCleared = false;

	// Check head
	if (!InOutProfile.AppearanceData.HeadId.IsNone())
	{
		if (!Catalog->FindHeadEntryForContext(InOutProfile.AppearanceData.HeadId, InOutProfile.Gender, InOutProfile.Race))
		{
			UE_LOG(LogTemp, Log, TEXT("TainlordCreation: Clearing head '%s' - not valid for new context (Gender=%d, Race=%d)"),
				*InOutProfile.AppearanceData.HeadId.ToString(),
				static_cast<int32>(InOutProfile.Gender),
				static_cast<int32>(InOutProfile.Race));
			InOutProfile.AppearanceData.HeadId = NAME_None;
			bAnyCleared = true;
		}
	}

	// Check hair
	if (!InOutProfile.AppearanceData.HairId.IsNone())
	{
		if (!Catalog->FindHairEntryForContext(InOutProfile.AppearanceData.HairId, InOutProfile.Gender, InOutProfile.Race))
		{
			UE_LOG(LogTemp, Log, TEXT("TainlordCreation: Clearing hair '%s' - not valid for new context"),
				*InOutProfile.AppearanceData.HairId.ToString());
			InOutProfile.AppearanceData.HairId = NAME_None;
			bAnyCleared = true;
		}
	}

	// Check beard
	if (!InOutProfile.AppearanceData.BeardId.IsNone())
	{
		if (!Catalog->FindBeardEntryForContext(InOutProfile.AppearanceData.BeardId, InOutProfile.Gender, InOutProfile.Race))
		{
			UE_LOG(LogTemp, Log, TEXT("TainlordCreation: Clearing beard '%s' - not valid for new context"),
				*InOutProfile.AppearanceData.BeardId.ToString());
			InOutProfile.AppearanceData.BeardId = NAME_None;
			bAnyCleared = true;
		}
	}

	// Skin tones are not gender/race gated — no sanitization needed

	return bAnyCleared;
}

// ---------------------------------------------------------------------------
// Confirm / save
// ---------------------------------------------------------------------------

bool UTainlordCharacterCreationLibrary::ConfirmCreation(const UObject* WorldContext, const FTainlordProfileData& FinalProfile, FString& OutReason)
{
	// Step 1: Validate the profile
	if (!ValidateProfile(FinalProfile, OutReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: ConfirmCreation failed validation - %s"), *OutReason);
		return false;
	}

	// Step 2: Get the profile subsystem
	if (!WorldContext)
	{
		OutReason = TEXT("No world context available.");
		return false;
	}

	const UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		OutReason = TEXT("Cannot resolve world from context.");
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		OutReason = TEXT("Cannot get GameInstance.");
		return false;
	}

	UTainlordCharacterProfileSubsystem* ProfileSubsystem = GameInstance->GetSubsystem<UTainlordCharacterProfileSubsystem>();
	if (!ProfileSubsystem)
	{
		OutReason = TEXT("Profile subsystem not found.");
		return false;
	}

	// Step 3: Set the active profile
	ProfileSubsystem->SetActiveProfile(FinalProfile);

	// Step 4: Save to the default slot
	if (!ProfileSubsystem->SaveActiveProfile(ProfileSubsystem->DefaultSlotName, ProfileSubsystem->DefaultUserIndex))
	{
		OutReason = TEXT("Failed to save profile to disk.");
		// Profile is still set as active in memory even if save failed
		UE_LOG(LogTemp, Warning, TEXT("TainlordCreation: Profile set as active but disk save failed"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TainlordCreation: Character creation confirmed "
		"(name=%s, gender=%d, race=%d, mastery=%s, city=%s)"),
		*FinalProfile.CharacterName,
		static_cast<int32>(FinalProfile.Gender),
		static_cast<int32>(FinalProfile.Race),
		*FinalProfile.SelectedMasteryId.ToString(),
		*FinalProfile.SelectedCityId.ToString());

	OutReason.Empty();
	return true;
}
