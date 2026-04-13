// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCharacterProfileSaveGame.h"
#include "Kismet/GameplayStatics.h"

UTainlordCharacterProfileSaveGame::UTainlordCharacterProfileSaveGame()
	: SlotName(TEXT("Character_0"))
	, UserIndex(0)
{
}

bool UTainlordCharacterProfileSaveGame::SaveProfile(const FTainlordProfileData& InProfileData, const FString& InSlotName, int32 InUserIndex)
{
	UTainlordCharacterProfileSaveGame* SaveGameInstance = Cast<UTainlordCharacterProfileSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UTainlordCharacterProfileSaveGame::StaticClass()));

	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordProfile: Failed to create save game object for slot '%s'"), *InSlotName);
		return false;
	}

	SaveGameInstance->ProfileData = InProfileData;
	SaveGameInstance->SlotName = InSlotName;
	SaveGameInstance->UserIndex = InUserIndex;

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGameInstance, InSlotName, InUserIndex);

	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Saved profile '%s' to slot '%s' "
			"(name=%s, gender=%d, race=%d, build=%s, mastery=%s, city=%s)"),
			*InProfileData.CharacterName,
			*InSlotName,
			*InProfileData.CharacterName,
			static_cast<int32>(InProfileData.Gender),
			static_cast<int32>(InProfileData.Race),
			*InProfileData.SelectedBuildId.ToString(),
			*InProfileData.SelectedMasteryId.ToString(),
			*InProfileData.SelectedCityId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordProfile: Failed to save profile to slot '%s'"), *InSlotName);
	}

	return bSaved;
}

bool UTainlordCharacterProfileSaveGame::LoadProfile(const FString& InSlotName, int32 InUserIndex, FTainlordProfileData& OutProfileData)
{
	if (!UGameplayStatics::DoesSaveGameExist(InSlotName, InUserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordProfile: No save found for slot '%s'"), *InSlotName);
		return false;
	}

	USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(InSlotName, InUserIndex);
	UTainlordCharacterProfileSaveGame* ProfileSave = Cast<UTainlordCharacterProfileSaveGame>(LoadedSave);

	if (!ProfileSave)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordProfile: Failed to load/cast save from slot '%s'"), *InSlotName);
		return false;
	}

	OutProfileData = ProfileSave->ProfileData;

	UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Loaded profile from slot '%s' "
		"(name=%s, gender=%d, race=%d, build=%s, mastery=%s, city=%s)"),
		*InSlotName,
		*OutProfileData.CharacterName,
		static_cast<int32>(OutProfileData.Gender),
		static_cast<int32>(OutProfileData.Race),
		*OutProfileData.SelectedBuildId.ToString(),
		*OutProfileData.SelectedMasteryId.ToString(),
		*OutProfileData.SelectedCityId.ToString());

	return true;
}

bool UTainlordCharacterProfileSaveGame::DoesSaveExist(const FString& InSlotName, int32 InUserIndex)
{
	return UGameplayStatics::DoesSaveGameExist(InSlotName, InUserIndex);
}

bool UTainlordCharacterProfileSaveGame::DeleteProfile(const FString& InSlotName, int32 InUserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(InSlotName, InUserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordProfile: No save to delete for slot '%s'"), *InSlotName);
		return false;
	}

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(InSlotName, InUserIndex);

	if (bDeleted)
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordProfile: Deleted profile from slot '%s'"), *InSlotName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordProfile: Failed to delete profile from slot '%s'"), *InSlotName);
	}

	return bDeleted;
}
