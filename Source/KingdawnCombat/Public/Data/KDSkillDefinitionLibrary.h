// Copyright Kingdawn. All Rights Reserved.
// C++ library for creating and querying skill definition packages.
// Provides factory methods for branch-specific skill sets.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/KDBranchTypes.h"
#include "UObject/SoftObjectPath.h"
#include "KDSkillDefinitionLibrary.generated.h"

class UKDAbilityDefinition;

/**
 * Static library for skill definition creation and querying.
 * 
 * Usage:
 * - CreateBladeShieldPackage() creates all 4 BladeShield skill definitions at runtime
 * - FindDefinitionByTag() looks up a definition by its ability tag
 * - GetBranchSkills() returns all definitions for a given branch
 * - IsSkillCompatible() validates a definition against a class/branch/weapon combo
 * 
 * Skill definitions are UPrimaryDataAsset objects. This library creates them
 * as transient runtime objects for testing, or they can be authored as Blueprint
 * data assets in the Content Browser for production.
 */
UCLASS()
class KINGDAWNCOMBAT_API UKDSkillDefinitionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// --- Package Factories ---

	/**
	 * Create the complete BladeShield skill definition package.
	 * Returns 4 definitions: BasicAttack, ComboLight, HeavyStrike, ShieldBash.
	 * These are transient runtime objects for testing/validation.
	 * Production use should author these as data assets in Content Browser.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|SkillDefinitions")
	static TArray<UKDAbilityDefinition*> CreateBladeShieldPackage(UObject* Outer);

	/**
	 * Load a persistent KDAbilityDefinition asset by soft path.
	 * Returns null if the asset cannot be loaded or is not a KDAbilityDefinition.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|SkillDefinitions")
	static UKDAbilityDefinition* LoadDefinitionByPath(const FSoftObjectPath& AssetPath);

	/**
	 * Load the production BladeShield package from authored data assets.
	 * Missing assets are skipped so callers can handle partial content safely.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|SkillDefinitions")
	static TArray<UKDAbilityDefinition*> LoadBladeShieldPackageFromAssets();

	/**
	 * Create the complete Wizard skill definition package.
	 * Returns 4 definitions: ArcaneBolt, Fireball, FrostNova, ManaShield.
	 * These are transient runtime objects for testing/validation.
	 * Production use should author these as data assets in Content Browser.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|SkillDefinitions")
	static TArray<UKDAbilityDefinition*> CreateWizardPackage(UObject* Outer);

	/**
	 * Load the production Wizard package from authored data assets.
	 * Missing assets are skipped so callers can handle partial content safely.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|SkillDefinitions")
	static TArray<UKDAbilityDefinition*> LoadWizardPackageFromAssets();

	// --- Query Helpers ---

	/**
	 * Find a skill definition by its ability tag from a list.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|SkillDefinitions")
	static UKDAbilityDefinition* FindDefinitionByTag(const TArray<UKDAbilityDefinition*>& Definitions, FGameplayTag AbilityTag);

	// --- Legacy Compatibility Queries ---

	/**
	 * Check if a skill definition is compatible with the given class/branch/weapon setup.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|SkillDefinitions")
	static bool IsSkillCompatible(const UKDAbilityDefinition* Definition, EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon);

	/**
	 * Filter a list of definitions to only those compatible with the given setup.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|SkillDefinitions")
	static TArray<UKDAbilityDefinition*> FilterCompatibleSkills(const TArray<UKDAbilityDefinition*>& Definitions, EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon);

	// --- Mastery-Aware Compatibility Queries ---

	/**
	 * Check if a skill definition is compatible with the given mastery identity and weapon.
	 * Preferred API for new mastery-system code.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|SkillDefinitions")
	static bool IsSkillCompatibleByMastery(const UKDAbilityDefinition* Definition, FKDMasteryIdentity MasteryId, EKDWeaponType Weapon);

	/**
	 * Filter a list of definitions to only those compatible with the given mastery identity and weapon.
	 * Preferred API for new mastery-system code.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|SkillDefinitions")
	static TArray<UKDAbilityDefinition*> FilterCompatibleSkillsByMastery(const TArray<UKDAbilityDefinition*>& Definitions, FKDMasteryIdentity MasteryId, EKDWeaponType Weapon);

private:
	/** Internal: Create a single BladeShield skill definition with common defaults. */
	static UKDAbilityDefinition* CreateBladeShieldSkill(UObject* Outer, const FString& Name);

	/** Internal: Create a single Wizard skill definition with common defaults. */
	static UKDAbilityDefinition* CreateWizardSkill(UObject* Outer, const FString& Name);
};
