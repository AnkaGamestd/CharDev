// Copyright Kingdawn. All Rights Reserved.
// Status effect ability shell for FrostNova and similar CC spells.
// Applies GameplayEffect-based slow/freeze to targets within AoE radius.
// Only status effects cause interruption - normal damage does NOT interrupt.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/KDGA_RangedSpell.h"
#include "KDGA_StatusEffect.generated.h"

class UAbilitySystemComponent;

/**
 * Status effect ability for the Wizard branch.
 * Extends KDGA_RangedSpell with:
 * - AoE radius targeting (ground-targeted or self-centered)
 * - GameplayEffect application for status effects (slow, freeze, stun)
 * - Duration-based effect control
 * 
 * FrostNova: self-centered AoE slow/freeze, no projectile needed.
 * Future: can be reused for other CC spells.
 */
UCLASS(Abstract, Blueprintable)
class KINGDAWNCOMBAT_API UKDGA_StatusEffect : public UKDGA_RangedSpell
{
	GENERATED_BODY()

public:
	UKDGA_StatusEffect();

	// --- Status Effect Config ---

	/** Radius of the AoE effect. 0 = single target (uses base ranged spell behavior). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|StatusEffect")
	float AoERadius;

	/** Whether the AoE is centered on the caster (true) or on the target location (false). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|StatusEffect")
	bool bSelfCentered;

	/** GameplayEffect to apply as the status effect (slow, freeze, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|StatusEffect")
	TSubclassOf<class UGameplayEffect> StatusEffect;

	/** Duration of the status effect in seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|StatusEffect")
	float EffectDuration;

	/** Magnitude of the effect (e.g., slow percentage: 0.3 = 30% speed reduction). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|StatusEffect")
	float EffectMagnitude;

	/** Tag to identify the status effect type for immunity/stacking checks. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|StatusEffect")
	FGameplayTag StatusEffectTag;

protected:
	/** Override ActivateAbility to bypass montage-dependent base class chain.
	 *  Status effects are self-centered AoE with no montage requirement.
	 *  Handles commit, fires spell, and ends ability in one clean pass. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/** Override FireSpell to apply AoE status effect instead of single-target damage. */
	virtual void FireSpell_Implementation() override;

	/** Check if a target is immune to the current status effect based on immunity tags. */
	bool IsTargetImmune(AActor* Target, UAbilitySystemComponent* TargetASC);

	/** Apply the status effect GameplayEffect to a validated target. */
	void ApplyStatusEffectGEToTarget(AActor* Target, UAbilitySystemComponent* TargetASC);

	/**
	 * Blueprint hook for visual effects when the AoE triggers.
	 * Spawn VFX, particle systems, etc.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kingdawn|StatusEffect", DisplayName = "OnStatusEffectApplied")
	void BP_OnStatusEffectApplied(const FVector& Center, float Radius);
};
