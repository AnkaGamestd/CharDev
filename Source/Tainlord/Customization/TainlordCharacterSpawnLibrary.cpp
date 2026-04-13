// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCharacterSpawnLibrary.h"
#include "Customization/TainlordCharacterAppearanceComponent.h"
#include "Customization/TainlordCharacterProfileSubsystem.h"
#include "Engine/GameInstance.h"

// ---------------------------------------------------------------------------
// ApplyActiveProfileToCharacter
// ---------------------------------------------------------------------------

bool UTainlordCharacterSpawnLibrary::ApplyActiveProfileToCharacter(AActor* Character, const UObject* WorldContext)
{
	if (!Character || !WorldContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: ApplyActiveProfileToCharacter - null Character or WorldContext"));
		return false;
	}

	// Get the profile subsystem from the game instance
	const UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: Cannot get World from context"));
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: Cannot get GameInstance"));
		return false;
	}

	UTainlordCharacterProfileSubsystem* ProfileSubsystem = GameInstance->GetSubsystem<UTainlordCharacterProfileSubsystem>();
	if (!ProfileSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: TainlordCharacterProfileSubsystem not found"));
		return false;
	}

	if (!ProfileSubsystem->HasActiveProfile())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: No active profile loaded - character will have default appearance"));
		return false;
	}

	const FTainlordProfileData& Profile = ProfileSubsystem->GetActiveProfile();
	return ApplyProfileToCharacter(Character, Profile);
}

// ---------------------------------------------------------------------------
// ApplyProfileToCharacter (the single authoritative apply point)
// ---------------------------------------------------------------------------

bool UTainlordCharacterSpawnLibrary::ApplyProfileToCharacter(AActor* Character, const FTainlordProfileData& ProfileData)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: ApplyProfileToCharacter - null Character"));
		return false;
	}

	UTainlordCharacterAppearanceComponent* AppearanceComp = GetAppearanceComponent(Character);
	if (!AppearanceComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: Character '%s' has no TainlordCharacterAppearanceComponent"), *Character->GetName());
		return false;
	}

	// The single authoritative apply call.
	// Sets profile context (gender/race) and applies all appearance data through the catalog.
	// This is the same call used by creation preview and spawn path.
	AppearanceComp->ApplyAppearanceWithContext(
		ProfileData.AppearanceData,
		ProfileData.Gender,
		ProfileData.Race
	);

	UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: Applied profile to character '%s' (name=%s, gender=%d, race=%d, head=%s)"),
		*Character->GetName(),
		*ProfileData.CharacterName,
		static_cast<int32>(ProfileData.Gender),
		static_cast<int32>(ProfileData.Race),
		*ProfileData.AppearanceData.HeadId.ToString());

	return true;
}

// ---------------------------------------------------------------------------
// GetAppearanceComponent
// ---------------------------------------------------------------------------

UTainlordCharacterAppearanceComponent* UTainlordCharacterSpawnLibrary::GetAppearanceComponent(AActor* Character)
{
	if (!Character)
	{
		return nullptr;
	}

	return Character->FindComponentByClass<UTainlordCharacterAppearanceComponent>();
}
