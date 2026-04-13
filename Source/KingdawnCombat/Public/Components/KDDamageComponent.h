// Copyright Kingdawn. All Rights Reserved.
// Rewritten from AscentCombatFramework/Components/ACFDamageHandlerComponent.h
// Removed: ACFAnimInstance hit reactions, ACFDamageCalculation, ARSLevelingComponent
// Replaced: Hit reactions use PlayMontage() instead of AnimInstance layers

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KDDamageComponent.generated.h"

class UAnimMontage;
class UKDStatsComponent;

/** Damage event data passed to delegates. */
USTRUCT(BlueprintType)
struct FKDDamageEvent
{
	GENERATED_BODY()

public:
	FKDDamageEvent()
		: FinalDamage(0.f)
		, bIsCritical(false)
		, DamageReceiver(nullptr)
		, DamageDealer(nullptr)
		, DamageDirection(FVector::ZeroVector)
		, HitBoneName(NAME_None)
		, HitLocation(FVector::ZeroVector)
	{}

	/** Final damage after calculations. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	float FinalDamage;

	/** Whether this was a critical hit. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	bool bIsCritical;

	/** Actor that received the damage. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	TObjectPtr<AActor> DamageReceiver;

	/** Actor that dealt the damage. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	TObjectPtr<AActor> DamageDealer;

	/** Direction the damage came from. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	FVector DamageDirection;

	/** Bone that was hit. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	FName HitBoneName;

	/** World location of the hit. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	FVector HitLocation;

	/** Hit result from the trace. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	FHitResult HitResult;

	/** Gameplay tags associated with this damage. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	FGameplayTagContainer DamageTags;
};

// --- Delegates ---

/** Broadcast when the owning actor dies. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKDOnCharacterDeath);

/** Broadcast when the owning actor receives damage. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKDOnDamageReceived, const FKDDamageEvent&, DamageReceived);

/** Broadcast when the owning actor inflicts damage on another. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKDOnDamageInflicted, const FKDDamageEvent&, DamageInflicted);

/**
 * Damage handler component. Processes incoming damage, manages health reduction,
 * and handles death state.
 *
 * SILKROAD REMASTERED COMBAT RULE:
 * Normal damage does NOT interrupt abilities, stop montage playback, or force
 * hit reactions. Only explicit status effects (stun, knockback, CC) may interrupt.
 * This applies universally to PvP, PvE, elite fights, and boss fights.
 *
 * bPlayHitReactions is FALSE by default. If enabled (e.g., for trash mobs),
 * PlayHitReaction() will play a directional montage that DOES interrupt.
 * For status-effect-driven interrupts, call ForceHitReaction() directly.
 *
 * Key differences from ACF:
 * - No ACFAnimInstance dependency (uses direct montage playback)
 * - No ACFDamageCalculation (simple damage pass-through)
 * - No ARSLevelingComponent (no XP on kill)
 * - Hit reactions are opt-in, not universal
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDDamageComponent();

	// --- Delegates ---

	/** Broadcast when this actor receives damage. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnDamageReceived OnDamageReceived;

	/** Broadcast when this actor inflicts damage on another. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnDamageInflicted OnDamageInflicted;

	/** Broadcast when this actor dies. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDOnCharacterDeath OnOwnerDeath;

	// --- Getters ---

	/** Get the last damage event received. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	const FKDDamageEvent& GetLastDamageInfo() const { return LastDamageReceived; }

	/** Check if the actor is alive. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	bool GetIsAlive() const { return bIsAlive; }

	/** Check if the actor is immortal. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	bool GetIsImmortal() const { return bIsImmortal; }

	/** Set immortality. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void SetIsImmortal(bool bInImmortal) { bIsImmortal = bInImmortal; }

	// --- Damage Processing ---

	/**
	 * Centralized damage calculation v1 - applies attacker/defender stat scaling.
	 * This is the main entry point for all damage in the combat system.
	 * Handles: Physical/Arcane/Hybrid scaling, mitigation, critical hits (v1 placeholder).
	 *
	 * @param RawDamage Base damage value before scaling
	 * @param DamageCauser Actor dealing the damage (attacker)
	 * @param DamagedActor Actor receiving the damage (defender)
	 * @param InstigatedBy Controller of the attacker (optional)
	 * @return Final damage after all calculations
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Damage")
	static float CalculateDamage(float RawDamage, AActor* DamageCauser, AActor* DamagedActor,
		AController* InstigatedBy = nullptr);

	/**
	 * Internal version of CalculateDamage that outputs damage tags.
	 * Used by ApplyAbilityDamage to populate damage event with proper tags.
	 */
	static float CalculateDamageWithTags(float RawDamage, AActor* DamageCauser, AActor* DamagedActor,
		AController* InstigatedBy, FGameplayTagContainer& OutDamageTags);

	/**
	 * Apply ability/spell damage through the full damage pipeline.
	 * This is the correct entry point for spell and AoE damage.
	 *
	 * Pipeline: CalculateDamage → immortality guard → LastDamageReceived →
	 *           OnDamageReceived → dealer OnDamageInflicted → StatsComponent::ReceiveDamage
	 *
	 * Abilities should call this instead of directly calling KDStatsComponent::ReceiveDamage().
	 * The raw damage is passed through CalculateDamage() for stat scaling, then
	 * the full event pipeline runs (immortality check, delegates, death handling).
	 *
	 * @param RawDamage Base damage before formula scaling
	 * @param DamageCauser Actor dealing the damage (attacker)
	 * @param InstigatedBy Controller of the attacker (optional)
	 * @return Final damage applied after all calculations, or 0 if blocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Damage")
	float ApplyAbilityDamage(float RawDamage, AActor* DamageCauser, AController* InstigatedBy = nullptr);

	/**
	 * Process point damage (melee hits, projectiles).
	 * Called from the character's TakeDamage or from collision component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	float TakePointDamage(float Damage, FVector HitLocation, FVector HitNormal,
		FName BoneName, FVector ShotFromDirection,
		AController* InstigatedBy, AActor* DamageCauser, const FHitResult& HitInfo);

	/**
	 * Process generic damage (explosions, AOE, etc).
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	float TakeDamage(AActor* DamageReceiver, float Damage, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser);

	/**
	 * Revive the actor (restore alive state, refill health).
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void Revive();

	/**
	 * Trigger death immediately (e.g., from script or ability).
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void TriggerDeath();

	// --- Hit Reaction Configuration ---

	/** Montage to play when hit from the front. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|HitReactions")
	TObjectPtr<UAnimMontage> HitReactionFrontMontage;

	/** Montage to play when hit from the back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|HitReactions")
	TObjectPtr<UAnimMontage> HitReactionBackMontage;

	/** Montage to play when hit from the left. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|HitReactions")
	TObjectPtr<UAnimMontage> HitReactionLeftMontage;

	/** Montage to play when hit from the right. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|HitReactions")
	TObjectPtr<UAnimMontage> HitReactionRightMontage;

	/** Default hit reaction montage (fallback). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|HitReactions")
	TObjectPtr<UAnimMontage> DefaultHitReactionMontage;

	/** Whether normal damage auto-plays hit reaction montages.
	 *  FALSE by default (Silkroad combat rule: no universal interrupt).
	 *  Set TRUE only for specific cases like trash mobs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|HitReactions")
	bool bPlayHitReactions;

	/**
	 * Force a hit reaction (for status effects like stun, knockback).
	 * This is the ONLY correct way to interrupt a victim's action from damage.
	 * Normal damage code should NOT call this.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|HitReactions")
	void ForceHitReaction(UAnimMontage* OverrideMontage = nullptr);

	/** Whether to enable ragdoll on death. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Death")
	bool bEnableRagdollOnDeath;

protected:
	virtual void BeginPlay() override;

private:
	/** Whether the actor is currently alive. */
	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	bool bIsAlive;

	/** Whether the actor is immortal (cannot take damage). */
	UPROPERTY()
	bool bIsImmortal;

	/** Last received damage event. */
	UPROPERTY(ReplicatedUsing = OnRep_LastDamageReceived)
	FKDDamageEvent LastDamageReceived;

	/** Construct a damage event from raw damage data. */
	void ConstructDamageEvent(AActor* DamagedActor, float Damage, AController* InstigatedBy,
		const FHitResult& HitInfo, FVector ShotFromDirection,
		const FDamageEvent& DamageEvent, AActor* DamageCauser);

	/** Centralized deterministic damage formula for runtime damage processing. */
	float CalculateFinalDamage(float RawDamage, AActor* DamagedActor, AController* InstigatedBy,
		AActor* DamageCauser, const FDamageEvent& DamageEvent, const FHitResult& HitInfo,
		bool& bOutIsCritical, FGameplayTagContainer& OutDamageTags) const;

	/** Play the appropriate hit reaction montage based on damage direction. */
	void PlayHitReaction(const FKDDamageEvent& DamageEvent);

	/** Handle death when health reaches zero. */
	UFUNCTION()
	void HandleStatReachedZero();

	UFUNCTION()
	void OnRep_IsAlive();

	UFUNCTION()
	void OnRep_LastDamageReceived();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
