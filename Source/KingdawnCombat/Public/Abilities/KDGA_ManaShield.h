// Copyright Kingdawn. All Rights Reserved.
// ManaShield ability shell for Wizard branch.
// Self-buff that absorbs damage using mana pool.
// Pure GameplayEffect-based buff - no projectile, no target needed.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/KDGameplayAbility.h"
#include "KDGA_ManaShield.generated.h"

/**
 * Mana Shield ability for the Wizard branch.
 * Activates a self-buff that absorbs incoming damage, consuming mana instead of health.
 * 
 * Design notes (Silkroad-style):
 * - Self-cast, no target required
 * - Absorbs damage based on remaining mana
 * - Ends when mana is depleted or duration expires
 * - Does NOT interrupt normal combat flow (buff, not CC)
 */
UCLASS(Abstract, Blueprintable)
class KINGDAWNCOMBAT_API UKDGA_ManaShield : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGA_ManaShield();

	// --- Mana Shield Config ---

	/** GameplayEffect to apply as the shield buff. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|ManaShield")
	TSubclassOf<class UGameplayEffect> ShieldEffect;

	/** Duration of the shield in seconds. 0 = until mana depleted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|ManaShield")
	float ShieldDuration;

	/** Percentage of damage absorbed by mana (0-1). 1 = 100% absorbed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|ManaShield")
	float AbsorptionRatio;

	/** Maximum damage the shield can absorb before breaking. 0 = unlimited (mana-gated). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|ManaShield")
	float MaxAbsorption;

	// --- Lifecycle ---

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Apply the shield buff to self. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|ManaShield")
	void ApplyShieldBuff();

	/** Remove the shield buff. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|ManaShield")
	void RemoveShieldBuff();

	/**
	 * Blueprint hook for shield VFX (aura, particle system).
	 * Called when the shield is activated.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kingdawn|ManaShield", DisplayName = "OnShieldActivated")
	void BP_OnShieldActivated();

	/**
	 * Blueprint hook for shield removal VFX.
	 * Called when the shield expires or is cancelled.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kingdawn|ManaShield", DisplayName = "OnShieldDeactivated")
	void BP_OnShieldDeactivated();

private:
	/** Handle for the applied shield GE, used for removal. */
	FActiveGameplayEffectHandle ShieldEffectHandle;
};
