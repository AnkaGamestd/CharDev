// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCharacterProfileSubsystem.h"
#include "Customization/TainlordCharacterProfileSaveGame.h"

void UTainlordCharacterProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Subsystem initialized"));

	// Auto-load default profile on startup
	AttemptAutoLoad();
}

void UTainlordCharacterProfileSubsystem::Deinitialize()
{
	// Save active profile on shutdown if one is loaded
	if (HasActiveProfile())
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Deinitializing - saving active profile to slot '%s'"), *DefaultSlotName);
		UTainlordCharacterProfileSaveGame::SaveProfile(ActiveProfile, DefaultSlotName, DefaultUserIndex);
	}

	Super::Deinitialize();
}

bool UTainlordCharacterProfileSubsystem::HasActiveProfile() const
{
	// A profile is considered active if it has any meaningful data
	return ActiveProfile.HasAnyData();
}

bool UTainlordCharacterProfileSubsystem::LoadProfileFromSlot(const FString& SlotName, int32 UserIndex)
{
	FTainlordProfileData LoadedProfile;
	const bool bSuccess = UTainlordCharacterProfileSaveGame::LoadProfile(SlotName, UserIndex, LoadedProfile);

	if (bSuccess)
	{
		ActiveProfile = LoadedProfile;
		UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Loaded profile from slot '%s' "
			"(name=%s, gender=%d, race=%d, mastery=%s, city=%s)"),
			*SlotName,
			*ActiveProfile.CharacterName,
			static_cast<int32>(ActiveProfile.Gender),
			static_cast<int32>(ActiveProfile.Race),
			*ActiveProfile.SelectedMasteryId.ToString(),
			*ActiveProfile.SelectedCityId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordProfile: Failed to load profile from slot '%s'"), *SlotName);
	}

	return bSuccess;
}

bool UTainlordCharacterProfileSubsystem::SaveActiveProfile(const FString& SlotName, int32 UserIndex)
{
	if (!HasActiveProfile())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordProfile: No active profile to save"));
		return false;
	}

	return UTainlordCharacterProfileSaveGame::SaveProfile(ActiveProfile, SlotName, UserIndex);
}

void UTainlordCharacterProfileSubsystem::SetActiveProfile(const FTainlordProfileData& NewProfile)
{
	ActiveProfile = NewProfile;
	UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Active profile set "
		"(name=%s, gender=%d, race=%d, mastery=%s, city=%s)"),
		*ActiveProfile.CharacterName,
		static_cast<int32>(ActiveProfile.Gender),
		static_cast<int32>(ActiveProfile.Race),
		*ActiveProfile.SelectedMasteryId.ToString(),
		*ActiveProfile.SelectedCityId.ToString());
}

void UTainlordCharacterProfileSubsystem::ClearActiveProfile()
{
	ActiveProfile = FTainlordProfileData();
	UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Active profile cleared"));
}

void UTainlordCharacterProfileSubsystem::AttemptAutoLoad()
{
	UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Attempting auto-load from default slot '%s'..."), *DefaultSlotName);

	if (UTainlordCharacterProfileSaveGame::DoesSaveExist(DefaultSlotName, DefaultUserIndex))
	{
		if (LoadProfileFromSlot(DefaultSlotName, DefaultUserIndex))
		{
			UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Auto-load succeeded"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TainlordProfile: Auto-load failed - save exists but could not be loaded"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordProfile: No save found in default slot '%s' - triggering creation flow"), *DefaultSlotName);

		// Broadcast the creation flow delegate so Blueprint/C++ listeners can open creation UI.
		// The delegate is broadcast on the game thread during subsystem initialization.
		if (OnCreationFlowNeeded.IsBound())
		{
			OnCreationFlowNeeded.Broadcast();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TainlordProfile: OnCreationFlowNeeded has no bindings - creation UI will not open. "
				"Bind to OnCreationFlowNeeded on the profile subsystem to handle character creation."));
		}
	}
}
