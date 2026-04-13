// Copyright Kingdawn. All Rights Reserved.
// Base ranged spell ability for Wizard branch.
// Handles: mana cost, ranged target validation, projectile spawn point, cast time placeholder.
// Subclassed by ArcaneBolt, Fireball, and similar ranged magic.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/KDGameplayAbility.h"
#include "KDGA_RangedSpell.generated.h"

/**
 * Base ranged spell ability for the Wizard branch.
 * Extends KDGameplayAbility with:
 * - Cast time support (montage-driven, not a separate timer)
 * - Projectile spawn hook (virtual, for subclass override or BP implementation)
 * - Longer default range (1500 vs melee 300)
 * - Mana cost defaults
 * 
 * ArcaneBolt and Fireball both derive from or use this shell.
 * No dodge, no iframe - Silkroad-style tab-target.
 */
UCLASS(Abstract, Blueprintable)
class KINGDAWNCOMBAT_API UKDGA_RangedSpell : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGA_RangedSpell();

	// --- Ranged Spell Config ---

	/** Base damage amount for this spell. Overridden by definition if assigned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Spell")
	float BaseDamage;

	/** Projectile speed (units/sec). 0 = instant hit (like ArcaneBolt). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Spell")
	float ProjectileSpeed;

	/** Whether this spell uses a projectile actor or applies damage instantly at range. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Spell")
	bool bUsesProjectile;

	/** Socket name on the character mesh to spawn projectiles from. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Spell")
	FName ProjectileSpawnSocket;

	/** Optional gameplay effect class to apply on hit (damage, debuff, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Spell")
	TSubclassOf<class UGameplayEffect> HitEffect;

	// --- Lifecycle ---

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/**
	 * Called at the montage event point to fire the spell.
	 * Default implementation applies instant damage to current target.
	 * Override for projectile spawning or AoE behavior.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Kingdawn|Spell")
	void FireSpell();
	virtual void FireSpell_Implementation();

	/**
	 * Apply spell damage to the target through the KDDamageComponent pipeline.
	 * Routes through ApplyAbilityDamage() for full event pipeline support
	 * (immortality guard, LastDamageReceived, OnDamageReceived, OnDamageInflicted).
	 * Falls back to direct stats damage if target has no KDDamageComponent.
	 *
	 * @param Target The actor to damage
	 * @param RawDamage Base damage before formula scaling
	 * @param DamageCauser The actor dealing the damage (typically the caster)
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Spell")
	void ApplySpellDamageToTarget(AActor* Target, float RawDamage, AActor* DamageCauser = nullptr);

	/** Montage event callback - fires the spell when the anim notify triggers. */
	UFUNCTION()
	void OnSpellEventReceived(FGameplayEventData Payload);
};
