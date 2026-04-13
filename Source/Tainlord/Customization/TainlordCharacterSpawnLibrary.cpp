// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCharacterSpawnLibrary.h"
#include "Customization/TainlordCharacterAppearanceComponent.h"
#include "Customization/TainlordCharacterProfileSubsystem.h"
#include "Combat/TainlordCombatRuleLibrary.h"
#include "Characters/KDCombatCharacter.h"
#include "Core/KDMasteryTypes.h"
#include "Mastery/KDMasteryComponent.h"
#include "Mastery/KDMasteryDefinition.h"
#include "Components/KDAbilitySystemComponent.h"
#include "Components/KDHotbarComponent.h"
#include "Data/KDBuildDefinition.h"
#include "Engine/GameInstance.h"
#include "UObject/SoftObjectPath.h"

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

	// V1 Mastery Bridge: Apply mastery selection to combat characters
	// This is a best-effort operation - appearance application succeeds even if mastery fails
	ApplyMasteryToCharacter(Character, ProfileData.SelectedMasteryId);

	return true;
}

// ---------------------------------------------------------------------------
// ApplyMasteryToCharacter (v1 mastery bridge - full runtime integration)
// ---------------------------------------------------------------------------

namespace
{
	// Asset path mappings for build and mastery definitions
	// These are soft-loaded at runtime to avoid hard asset dependencies
	struct FMasteryAssetPaths
	{
		FName BuildPath;
		FName MasteryPath;
	};

	FMasteryAssetPaths GetAssetPathsForMastery(FName MasteryId)
	{
		// Full UE object path format: /PackagePath/AssetName.AssetName
		static const TMap<FName, FMasteryAssetPaths> AssetPathMap = {
			{ FName("Arcanist"), { FName("/Game/KingdawnCombat/Data/Builds/DA_Build_Wizard.DA_Build_Wizard"), FName("/Game/KingdawnCombat/Data/Data/Mastery/DA_Mastery_Wizard.DA_Mastery_Wizard") } },
			{ FName("Guardian"), { FName("/Game/KingdawnCombat/Data/Builds/DA_Build_BladeShield.DA_Build_BladeShield"), FName("/Game/KingdawnCombat/Data/Data/Mastery/DA_Mastery_BladeShield.DA_Mastery_BladeShield") } },
			{ FName("Berserker"), { FName("/Game/KingdawnCombat/Data/Builds/DA_Build_BladeShield.DA_Build_BladeShield"), FName("/Game/KingdawnCombat/Data/Data/Mastery/DA_Mastery_BladeShield.DA_Mastery_BladeShield") } },
		};

		if (const FMasteryAssetPaths* Paths = AssetPathMap.Find(MasteryId))
		{
			return *Paths;
		}
		return FMasteryAssetPaths();
	}

	UKDBuildDefinition* LoadBuildAsset(const FName& BuildPath)
	{
		if (BuildPath.IsNone()) return nullptr;
		return Cast<UKDBuildDefinition>(FSoftObjectPath(BuildPath.ToString()).TryLoad());
	}

	UKDMasteryDefinition* LoadMasteryAsset(const FName& MasteryPath)
	{
		if (MasteryPath.IsNone()) return nullptr;
		return Cast<UKDMasteryDefinition>(FSoftObjectPath(MasteryPath.ToString()).TryLoad());
	}
}

void UTainlordCharacterSpawnLibrary::ApplyMasteryToCharacter(AActor* Character, FName MasteryId)
{
	if (MasteryId.IsNone())
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: No mastery ID in profile - skipping mastery application"));
		return;
	}

	// Check if this is a valid demo mastery ID
	if (!UTainlordCombatRuleLibrary::IsValidDemoMasteryId(MasteryId))
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: Invalid mastery ID '%s' - skipping mastery application"), *MasteryId.ToString());
		return;
	}

	// Map mastery ID to FKDMasteryIdentity
	FKDMasteryIdentity MasteryIdentity = UTainlordCombatRuleLibrary::MapMasteryIdToCombatIdentity(MasteryId);
	if (!MasteryIdentity.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: Failed to map mastery ID '%s' to FKDMasteryIdentity"), *MasteryId.ToString());
		return;
	}

	// Try to apply to KDCombatCharacter if present
	AKDCombatCharacter* CombatCharacter = Cast<AKDCombatCharacter>(Character);
	if (!CombatCharacter)
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: Character '%s' is not a KDCombatCharacter - mastery application skipped"),
			*Character->GetName());
		return;
	}

	// Step 1: Set mastery identity on the character
	CombatCharacter->SetMasteryIdentity(MasteryIdentity);
	UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: Set mastery identity '%s' on '%s' (Class=%d, Branch=%d)"),
		*MasteryId.ToString(),
		*Character->GetName(),
		static_cast<int32>(MasteryIdentity.MasteryClass),
		static_cast<int32>(MasteryIdentity.MasteryBranch));

	// Step 2: Apply build definition if available
	const FMasteryAssetPaths AssetPaths = GetAssetPathsForMastery(MasteryId);
	if (!AssetPaths.BuildPath.IsNone())
	{
		if (UKDBuildDefinition* BuildDef = LoadBuildAsset(AssetPaths.BuildPath))
		{
			CombatCharacter->ApplyBuildDefinition(BuildDef);
			UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: Applied build '%s' to '%s'"), *AssetPaths.BuildPath.ToString(), *Character->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: Failed to load build asset '%s'"), *AssetPaths.BuildPath.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: No build asset configured for mastery '%s' (identity-only mode)"), *MasteryId.ToString());
	}

	// Step 3: Activate mastery definition if available
	if (!AssetPaths.MasteryPath.IsNone())
	{
		if (UKDMasteryDefinition* MasteryDef = LoadMasteryAsset(AssetPaths.MasteryPath))
		{
			if (UKDMasteryComponent* MasteryComp = CombatCharacter->FindComponentByClass<UKDMasteryComponent>())
			{
				MasteryComp->ActivateMastery(MasteryIdentity, MasteryDef);
				UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: Activated mastery '%s' on '%s'"), *AssetPaths.MasteryPath.ToString(), *Character->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TainlordSpawn: Failed to load mastery asset '%s'"), *AssetPaths.MasteryPath.ToString());
		}
	}

	// Step 4: Reconcile abilities granted by mastery
	if (UKDAbilitySystemComponent* ASC = CombatCharacter->FindComponentByClass<UKDAbilitySystemComponent>())
	{
		const FKDMasteryReconciliationResult ReconciliationResult = ASC->ReconcileMasteryGrantedAbilities();
		UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: Ability reconciliation for '%s' - Granted=%d, Revoked=%d, Failed=%d"),
			*Character->GetName(),
			ReconciliationResult.GrantedSkills.Num(),
			ReconciliationResult.RevokedSkills.Num(),
			ReconciliationResult.FailedSkills.Num());
	}

	// Step 5: Reconcile hotbar if available
	if (UKDHotbarComponent* HotbarComp = CombatCharacter->FindComponentByClass<UKDHotbarComponent>())
	{
		const int32 PlacedSkills = HotbarComp->ReconcileMasteryHotbar();
		UE_LOG(LogTemp, Log, TEXT("TainlordSpawn: Hotbar reconciliation for '%s' - Placed %d skills"), *Character->GetName(), PlacedSkills);
	}
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
