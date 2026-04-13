// Copyright Kingdawn. All Rights Reserved.
// Combo attack ability - plays the authored full combo on one hotbar press.

#pragma once

#include "Abilities/KDGameplayAbility.h"
#include "CoreMinimal.h"
#include "KDGA_ComboAttack.generated.h"

/**
 * Combo attack ability that plays a full authored combo montage on a single activation.
 *
 * Architecture:
 * - Hotbar press starts the combo skill once
 * - Runtime links montage sections in order
 * - The full combo plays without repeated key presses
 * - Repeated key presses while active are ignored
 *
 * Montage structure requirement:
 * - Named sections: "Attack_0", "Attack_1", "Attack_2", etc.
 * - Sections may remain unlinked in the asset
 * - Runtime links them in order at activation time
 */
UCLASS()
class KINGDAWNCOMBAT_API UKDGA_ComboAttack : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGA_ComboAttack();

	UFUNCTION(BlueprintPure, Category = "Kingdawn|Combo")
	int32 GetComboCounter() const;

	UFUNCTION(BlueprintPure, Category = "Kingdawn|Combo")
	bool IsNextSectionQueued() const { return bNextSectionQueued; }

	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combo")
	void ResetComboCounter();

	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combo")
	void ForceComboCounter(int32 Value);

	/** One-press autoplay model: repeated input while active is ignored/consumed. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combo")
	void SendComboInput();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Get the first montage section name. */
	UFUNCTION(BlueprintNativeEvent, Category = "Kingdawn|Combo")
	FName GetMontageSectionName();

	/** Reset combo state when the full montage completes. */
	virtual void OnMontageCompleted() override;

	/** Reset combo state when interrupted. */
	virtual void OnMontageInterrupted() override;

private:
	/** Retained for compatibility/debugging. */
	bool bNextSectionQueued;

	/** Tracks current section for logs/debug. */
	int32 CurrentSectionIndex;
};
