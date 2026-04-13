// Copyright Kingdawn. All Rights Reserved.
// Master data asset for a build-driven runtime setup.
// A build captures class, branch, weapon types, ability set, and visual
// profiles so that activating a single asset configures the entire combat
// character in one pass.
//
// Storage authority (dual-storage bridge period):
//
// PRIMARY (new builds write here):
//   MasteryIdentity -> FKDMasteryIdentity (class + branch)
//
// MIRROR (auto-populated when build is applied):
//   CombatClass     -> EKDCombatClass
//   CombatBranch    -> EKDCombatBranch
//
// INDEPENDENT (weapon identity, not mastery):
//   PrimaryWeaponType / OffhandWeaponType -> EKDWeaponType

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/KDBranchTypes.h"
#include "Core/KDMasteryTypes.h"
#include "KDBuildDefinition.generated.h"

class UKDWeaponVisualProfile;
class UKDHotbarLayoutDefinition;

/**
 * Master build definition for a combat character.
 *
 * Designers author one UKDBuildDefinition per playable build (e.g.
 * "BladeShield Warrior", "Wizard Fire").  At runtime the combat system
 * reads the definition to configure class, branch, weapon visuals,
 * starting abilities, and hotbar layout in a single step.
 *
 * Cross-references to other data assets use TSoftObjectPtr so that
 * only the builds actually in play are loaded into memory.
 */
UCLASS(BlueprintType)
class KINGDAWNCOMBAT_API UKDBuildDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UKDBuildDefinition();

	// --- Identity ---

	/** Unique tag that identifies this build (e.g. KD.Build.BladeShield). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Identity")
	FGameplayTag BuildId;

	/** Human-readable name shown in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Identity")
	FText DisplayName;

	/** Description for tooltips / build selection screen. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Identity")
	FText Description;

	// --- Mastery Identity (PRIMARY) ---

	/**
	 * Mastery identity for this build (class + branch pair).
	 * New builds should set this field. Legacy CombatClass/CombatBranch
	 * are auto-populated when the build is applied to a character.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Mastery")
	FKDMasteryIdentity MasteryIdentity;

	// --- Legacy Class & Branch (MIRROR) ---

	/**
	 * LEGACY MIRROR: Broad combat class family.
	 * Auto-populated from MasteryIdentity when build is applied.
	 * Existing data assets that set this directly will still work.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Class")
	EKDCombatClass CombatClass;

	/**
	 * LEGACY MIRROR: Specific branch within the class.
	 * Auto-populated from MasteryIdentity when build is applied.
	 * Existing data assets that set this directly will still work.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Class")
	EKDCombatBranch CombatBranch;

	/**
	 * Populate legacy mirror fields from mastery identity.
	 * Called when applying the build to a character.
	 */
	void SyncLegacyFromMastery();

	/**
	 * Populate mastery identity from legacy fields.
	 * Use for one-time migration of existing data assets.
	 */
	void SyncMasteryFromLegacy();

	// --- Weapons ---

	/** Primary weapon type equipped by this build. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Weapons")
	EKDWeaponType PrimaryWeaponType;

	/** Offhand weapon type (Shield, Wand, None for two-handed, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Weapons")
	EKDWeaponType OffhandWeaponType;

	// --- Abilities ---

	/** Gameplay tags for all abilities this build starts with. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Abilities")
	TArray<FGameplayTag> DefaultAbilities;

	/** Tag that resolves to the basic-attack ability for this build. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Abilities")
	FGameplayTag BasicAttackAbility;

	/** Tag that resolves to the combo ability for this build. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Abilities")
	FGameplayTag ComboAbility;

	// --- Visuals & Layout ---

	/** Visual profile for the primary weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Visuals")
	TSoftObjectPtr<UKDWeaponVisualProfile> PrimaryWeaponVisualProfile;

	/** Visual profile for the offhand weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Visuals")
	TSoftObjectPtr<UKDWeaponVisualProfile> OffhandWeaponVisualProfile;

	/** Hotbar layout to apply when this build activates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Layout")
	TSoftObjectPtr<UKDHotbarLayoutDefinition> DefaultHotbarLayout;
};
