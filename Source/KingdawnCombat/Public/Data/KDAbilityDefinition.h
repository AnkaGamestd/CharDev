// Copyright Kingdawn. All Rights Reserved.
// Data-driven ability definition for reducing manual Blueprint setup.
// Includes branch/weapon compatibility for multi-class skill system.
//
// Migration note:
// IsCompatibleWith() uses legacy EKDCombatClass/EKDCombatBranch.
// IsCompatibleWithMastery() uses FKDMasteryIdentity from KDMasteryTypes.h.
// Both coexist during the migration period. New code should prefer
// the mastery-aware overload.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/KDBranchTypes.h"
#include "KDAbilityDefinition.generated.h"

class UAnimMontage;
class UTexture2D;

/**
 * High-level ability definition type for routing and tooling.
 * Determines how the ability system treats this definition.
 */
UENUM(BlueprintType)
enum class EKDAbilityDefinitionType : uint8
{
	ActiveSkill UMETA(DisplayName = "Active Skill"),
	PassiveSkill UMETA(DisplayName = "Passive Skill"),
	ComboSkill UMETA(DisplayName = "Combo Skill"),
	BuffSkill UMETA(DisplayName = "Buff Skill"),
	ConsumableItem UMETA(DisplayName = "Consumable Item")
};

/**
 * Skill tier for progression gating.
 * Skills can be locked behind level/progression requirements.
 */
UENUM(BlueprintType)
enum class EKDSkillTier : uint8
{
	Common UMETA(DisplayName = "Common"),
	Advanced UMETA(DisplayName = "Advanced"),
	Expert UMETA(DisplayName = "Expert"),
	Master UMETA(DisplayName = "Master")
};

/**
 * Lightweight skill/ability definition.
 * 
 * Goal:
 * - keep runtime in C++
 * - let designers select assets/config once in data
 * - reduce repeated per-ability Blueprint wiring
 * - express class/branch/weapon compatibility for multi-class system
 */
UCLASS(BlueprintType)
class KINGDAWNCOMBAT_API UKDAbilityDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UKDAbilityDefinition();

	// --- Identity ---

	/** Display-only label for UI and debugging. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Identity")
	FText DisplayName;

	/** Optional icon for hotbar/UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Identity")
	TObjectPtr<UTexture2D> Icon;

	/** Optional description for tooltip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Identity")
	FText Description;

	// --- Type ---

	/** High-level definition type for routing and tooling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Type")
	EKDAbilityDefinitionType DefinitionType;

	/** Skill tier for progression gating. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Type")
	EKDSkillTier SkillTier;

	// --- Ability Config ---

	/** Runtime ability identity tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	FGameplayTag AbilityTag;

	/** Montage to play for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	TObjectPtr<UAnimMontage> AbilityMontage;

	/** Stamina cost to activate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float StaminaCost;

	/** Mana cost to activate. 0 = no mana cost. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float ManaCost;

	/** Cooldown duration in seconds. 0 = no cooldown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float CooldownDuration;

	/** Whether activation commits cost/cooldown immediately. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	bool bCommitOnActivate;

	/** Whether montage completion should end the ability automatically. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	bool bEndOnMontageComplete;

	/** Montage play rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float MontagePlayRate;

	// --- Targeting ---

	/** Whether a valid target is required. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	bool bRequiresTarget;

	/** Minimum distance to target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	float MinRange;

	/** Maximum distance to target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	float MaxRange;

	/** Whether to automatically face target at activation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	bool bAutoFaceTarget;

	/** Rotation interpolation speed when facing target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	float FacingInterpSpeed;

	// --- Hotbar ---

	/** Whether this skill can be placed on the hotbar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Hotbar")
	bool bHotbarAssignable;

	// --- Compatibility ---

	/** Class/branch/weapon compatibility requirements. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Compatibility")
	FKDSkillCompatibility Compatibility;

	// --- Legacy Compatibility API ---

	/**
	 * Check if this skill definition is compatible with the given class/branch/weapon.
	 * Convenience wrapper around Compatibility.IsCompatible().
	 */
	bool IsCompatibleWith(EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon) const;

	// --- Mastery-Aware Compatibility API ---

	/**
	 * Check if this skill definition is compatible with the given mastery identity and weapon.
	 * Preferred API for new mastery-system code.
	 * Delegates to Compatibility.IsCompatibleWithMastery().
	 */
	bool IsCompatibleWithMastery(const FKDMasteryIdentity& MasteryId, EKDWeaponType Weapon) const;

	/**
	 * Check if this skill has a cooldown.
	 */
	bool HasCooldown() const { return CooldownDuration > 0.f; }
};
