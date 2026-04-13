// Copyright Kingdawn. All Rights Reserved.
// Rewritten from AdvancedRPGSystem/ARSStatisticsComponent.h
// Simplified: Health/Stamina only, no leveling, no XP, no stat modifiers.
// Uses KDCombatAttributeSet directly via GAS.

#pragma once

#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KDStatsComponent.generated.h"

class UKDCombatAttributeSet;
class UAbilitySystemComponent;

/** Broadcast when any tracked stat changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKDOnStatChanged, FGameplayTag, StatTag, float, NewValue);

/** Broadcast when health reaches zero. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKDOnHealthReachedZero);

/** Broadcast when the attribute set is modified. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKDOnAttributeSetModified);

/**
 * Lightweight stats component for Health and Stamina management.
 * Wraps GAS attribute set access with convenient Blueprint-callable functions.
 * No leveling, XP, or complex modifier system - just raw stat get/set/modify.
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDStatsComponent();

	// --- Delegates ---

	/** Broadcast when health reaches zero. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnHealthReachedZero OnHealthReachedZero;

	/** Broadcast when any attribute is modified. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnAttributeSetModified OnAttributeSetModified;

	/** Broadcast when a stat value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnStatChanged OnStatChanged;

	// --- Stat Getters ---

	/** Get current health. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetHealth() const;

	/** Get maximum health. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetMaxHealth() const;

	/** Get health as 0-1 normalized value. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetHealthNormalized() const;

	/** Get current stamina. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetStamina() const;

	/** Get maximum stamina. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetMaxStamina() const;

	/** Get stamina as 0-1 normalized value. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetStaminaNormalized() const;

	/** Get current mana. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetMana() const;

	/** Get maximum mana. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetMaxMana() const;

	/** Get mana as 0-1 normalized value. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	float GetManaNormalized() const;

	/** Check if the entity is alive (health > 0). */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	bool IsAlive() const;

	// --- Stat Modifiers ---

	/**
	 * Apply damage to health (reduces health by DamageAmount).
	 * Server only. Triggers OnHealthReachedZero if health reaches 0.
	 * @param DamageAmount Amount of damage to apply.
	 * @param Instigator The controller responsible for the damage.
	 * @param DamageCauser The actor that caused the damage.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void ReceiveDamage(float DamageAmount, AController* Instigator = nullptr, AActor* DamageCauser = nullptr);

	/**
	 * Consume stamina for an action (e.g., attack, block, dodge).
	 * @param Cost Amount of stamina to consume.
	 * @return True if stamina was sufficient and consumed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	bool ConsumeStamina(float Cost);

	/**
	 * Regenerate stamina over time. Call from Tick or a timer.
	 * @param Amount Amount to regenerate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void RegenerateStamina(float Amount);

	/** Fully restore health to max. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void RestoreHealth();

	/** Fully restore stamina to max. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void RestoreStamina();

	/** Fully restore mana to max. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void RestoreMana();

	/** Restore all stats to max. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void RestoreAllStats();

	/**
	 * Check if the entity can afford a stamina cost.
	 * @param Cost The stamina cost to check.
	 * @return True if current stamina >= Cost.
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	bool CanAffordStaminaCost(float Cost) const;

	/**
	 * Consume mana for a spell or magical ability.
	 * @param Cost Amount of mana to consume.
	 * @return True if mana was sufficient and consumed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	bool ConsumeMana(float Cost);

	/**
	 * Regenerate mana over time. Call from Tick or a timer.
	 * @param Amount Amount to regenerate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void RegenerateMana(float Amount);

	/**
	 * Check if the entity can afford a mana cost.
	 * @param Cost The mana cost to check.
	 * @return True if current mana >= Cost.
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Kingdawn|Stats")
	bool CanAffordManaCost(float Cost) const;

	// --- Initialization ---

	/**
	 * Initialize the stats component with an Ability System Component.
	 * Called automatically in BeginPlay if ASC is found on the owner.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Stats")
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InASC);

	// --- Configuration ---

	/** Stamina regeneration rate per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Stats|Config")
	float StaminaRegenRate;

	/** Delay after stamina consumption before regeneration begins (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Stats|Config")
	float StaminaRegenDelay;

		/** Whether stamina regeneration is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Stats|Config")
	bool bStaminaRegenEnabled;

	/** Mana regeneration rate per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Stats|Config")
	float ManaRegenRate;

	/** Delay after mana consumption before regeneration begins (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Stats|Config")
	float ManaRegenDelay;

	/** Whether mana regeneration is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Stats|Config")
	bool bManaRegenEnabled;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Cached pointer to the Ability System Component. */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Cached pointer to the combat attribute set. */
	UPROPERTY()
	TObjectPtr<UKDCombatAttributeSet> CombatAttributeSet;

	/** Time since last stamina consumption. */
	float TimeSinceLastStaminaUse;

	/** Whether the component has been initialized. */
	bool bInitialized;

	/** Internal: bind to ASC attribute change delegates. */
	void BindAttributeChangeDelegates();

	/** Internal: called when health attribute changes. */
	void OnHealthChanged(const struct FOnAttributeChangeData& ChangeData);

	/** Internal: called when stamina attribute changes. */
	void OnStaminaChanged(const struct FOnAttributeChangeData& ChangeData);

	/** Internal: called when mana attribute changes. */
	void OnManaChanged(const struct FOnAttributeChangeData& ChangeData);

	/** Internal: handle for stamina regen timer. */
	FTimerHandle StaminaRegenTimerHandle;

	/** Time since last mana consumption. */
	float TimeSinceLastManaUse;
};



