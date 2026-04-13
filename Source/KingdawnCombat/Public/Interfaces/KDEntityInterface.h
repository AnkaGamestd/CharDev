// Copyright Kingdawn. All Rights Reserved.
// Adapted from AscentCoreInterfaces/Interfaces/ACFEntityInterface.h
//
// Migration note:
// Legacy methods (GetEntityCombatClass, GetEntityCombatBranch) return legacy enums.
// New mastery-aware methods (GetEntityMasteryIdentity) return FKDMasteryIdentity.
// Both coexist during the bridge period. New code should prefer the mastery API.

#pragma once

#include "CoreMinimal.h"
#include "Core/KDCoreTypes.h"
#include "Core/KDBranchTypes.h"
#include "Core/KDMasteryTypes.h"
#include "UObject/Interface.h"
#include "KDEntityInterface.generated.h"

UINTERFACE(MinimalAPI)
class UKDEntityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any entity that participates in the combat system.
 * Provides team affiliation, extent radius, alive status, and combat identity.
 */
class KINGDAWNCOMBAT_API IKDEntityInterface
{
	GENERATED_BODY()

public:
	/** Returns the gameplay tag representing this entity's combat team. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Combat")
	FGameplayTag GetEntityCombatTeam() const;

	/** Returns the radius of a bounding sphere that encloses the entity mesh. Used for distance and warp calculations. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Combat")
	float GetEntityExtentRadius() const;

	/** Returns true if the entity is alive. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Combat")
	bool IsEntityAlive() const;

	/** Assigns a combat team tag to this entity. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Combat")
	void AssignTeamToEntity(FGameplayTag InCombatTeam);

	// --- Legacy Combat State (EKDCombatClass/EKDCombatBranch) ---

	/** Returns the entity's combat class family (Warrior, Mage, Rogue, Archer, Support). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Combat")
	EKDCombatClass GetEntityCombatClass() const;

	/** Returns the entity's combat branch (BladeShield, TwoHandedSword, Wizard, etc.). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Combat")
	EKDCombatBranch GetEntityCombatBranch() const;

	/** Returns the entity's primary weapon type (OneHandedSword, Staff, Bow, etc.). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Combat")
	EKDWeaponType GetEntityWeaponType() const;

	// --- Mastery-Aware Combat State (FKDMasteryIdentity) ---

	/**
	 * Returns the entity's mastery identity (class + branch pair).
	 * Preferred API for new mastery-system code.
	 * Default implementation converts legacy GetEntityCombatClass/GetEntityCombatBranch.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Kingdawn|Mastery")
	FKDMasteryIdentity GetEntityMasteryIdentity() const;
};
