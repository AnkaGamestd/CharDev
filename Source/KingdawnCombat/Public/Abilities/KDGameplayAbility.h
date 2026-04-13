// Copyright Kingdawn. All Rights Reserved.
// Base gameplay ability for MMO-style tab-target combat.
// Supports: range validation, target requirement, auto-facing, montage playback.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagContainer.h"
#include "KDGameplayAbility.generated.h"

class UKDAbilitySystemComponent;
class UKDCombatAttributeSet;
class UKDTargetingComponent;
class UKDAbilityDefinition;
class UAnimMontage;

/**
 * Base gameplay ability class for Kingdawn MMO combat.
 * Provides: montage playback, stamina cost, range validation, target requirement, auto-facing.
 * No dodge, no iframe system.
 */
UCLASS(Abstract, Blueprintable, HideCategories = "Input")
class KINGDAWNCOMBAT_API UKDGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility();

	// --- Configuration ---

	/** Optional data-driven definition. If assigned, its values override manual defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Definition")
	TObjectPtr<UKDAbilityDefinition> AbilityDefinition;

	/** Tag that identifies this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	FGameplayTag AbilityTag;

	/** Montage to play when this ability activates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	TObjectPtr<UAnimMontage> AbilityMontage;

	/** Stamina cost to activate this ability. 0 = free. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float StaminaCost;

	/** Mana cost to activate this ability. 0 = free. Used by Wizard/spell abilities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float ManaCost;

	/** Cooldown duration in seconds. 0 = no cooldown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float CooldownDuration;

	/** Whether to commit ability (cooldown + cost) on activation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	bool bCommitOnActivate;

	/** Whether to automatically end the ability when the montage completes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	bool bEndOnMontageComplete;

	/** Montage play rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Ability")
	float MontagePlayRate;

	// --- Targeting & Range (MMO-style) ---

	/** If true, this ability requires a valid target to activate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	bool bRequiresTarget;

	/** Minimum distance to target for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	float MinRange;

	/** Maximum distance to target for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	float MaxRange;

	/** If true, automatically face the target when ability activates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	bool bAutoFaceTarget;

	/** Speed of rotation interpolation when facing target. 0 = instant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Targeting")
	float FacingInterpSpeed;

	// --- Ability Lifecycle ---

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	// --- Montage Helpers ---

	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Ability")
	bool PlayAbilityMontage();

	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Ability")
	void StopAbilityMontage();

	UFUNCTION(BlueprintPure, Category = "Kingdawn|Ability")
	class UAnimInstance* GetOwnerAnimInstance() const;

	UFUNCTION(BlueprintPure, Category = "Kingdawn|Ability")
	UKDAbilitySystemComponent* GetKDAbilitySystemComponent() const;

	/** Get the effective cooldown duration for this ability. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Ability")
	float GetCooldownDuration() const;

	/** Copy config from the assigned ability definition into this ability instance/CDO. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Definition")
	void ApplyDefinitionOverrides();

	// --- Targeting Helpers ---

	/** Get the current target from the owning character's targeting component. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	AActor* GetAbilityTarget() const;

	/** Check if the current target is within the ability's range. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	bool IsTargetInRange() const;

	/** Face the current target. Called automatically if bAutoFaceTarget is true. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Targeting")
	void FaceTarget();

protected:
	UFUNCTION()
	virtual void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMontageInterrupted();

	/** Called when the montage is cancelled (e.g., failed to play on anim instance). */
	UFUNCTION()
	virtual void OnMontageCancelled();

	/** Called on montage blend-out; routes to OnMontageCompleted by default. */
	UFUNCTION()
	virtual void OnMontageBlendOut();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kingdawn|Ability", DisplayName = "OnMontageCompleted")
	void BP_OnMontageCompleted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Kingdawn|Ability", DisplayName = "OnMontageInterrupted")
	void BP_OnMontageInterrupted();

	/** Montage task - protected so combo abilities can access section jumping. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	bool bHasCommitted;
	bool bEndingAbility;
};
