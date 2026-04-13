// Copyright Kingdawn. All Rights Reserved.
// MMO-style combat character: tab-target, range-gated, auto-facing.
// No dodge, no iframe system.

#pragma once

#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/KDEntityInterface.h"
#include "Combat/KDWeaponFamilyTypes.h"
#include "KDCombatCharacter.generated.h"

class UKDBuildDefinition;
class UKDAbilitySystemComponent;
class UKDStatsComponent;
class UKDDamageComponent;
class UKDCollisionComponent;
class UKDTargetingComponent;
class UKDCombatAttributeSet;
class UKDFloorMarkerComponent;
class USkeletalMeshComponent;
class UTextRenderComponent;
class UKDWeaponVisualProfile;
class UKDMasteryComponent;

/**
 * Base combat character for Kingdawn MMO combat.
 * Owns all combat components including targeting.
 * Tab-target, range-gated, auto-facing attack model.
 */
UCLASS(Blueprintable)
class KINGDAWNCOMBAT_API AKDCombatCharacter : public ACharacter, public IAbilitySystemInterface, public IKDEntityInterface
{
	GENERATED_BODY()

public:
	AKDCombatCharacter();

	// --- IAbilitySystemInterface ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// --- IKDEntityInterface ---
	virtual FGameplayTag GetEntityCombatTeam_Implementation() const override;

	// --- Components ---

	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDAbilitySystemComponent* GetKDAbilitySystemComponent() const { return AbilitySystemComp; }

	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDStatsComponent* GetStatsComponent() const { return StatsComp; }

	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDDamageComponent* GetDamageComponent() const { return DamageComp; }

	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDCollisionComponent* GetCollisionComponent() const { return CollisionComp; }

	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDTargetingComponent* GetTargetingComponent() const { return TargetingComp; }

	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDFloorMarkerComponent* GetFloorMarkerComponent() const { return FloorMarkerComp; }

	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDMasteryComponent* GetMasteryComponent() const { return MasteryComp; }

	// --- Targeting ---

	/** Get the current combat target (may be null). */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	AActor* GetCurrentTarget() const;

	/** Set a combat target explicitly. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Targeting")
	void SetCombatTarget(AActor* NewTarget);

	/** Clear the current combat target. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Targeting")
	void ClearCombatTarget();

	// --- Combat Input ---

	/** Called when attack input is pressed. */
	void OnAttackInputPressed();

	/** Called when cycle target input is pressed. */
	void OnCycleTargetInputPressed();

	/** Called when clear target input is pressed. */
	void OnClearTargetInputPressed();

	/** Try to trigger an attack ability. Requires valid target in range. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combat")
	bool TryAttack();

	/**
	 * Try to activate any ability tag. If the ability requires a target and is out of range,
	 * the character auto-approaches to the correct range and then activates it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combat")
	bool ActivateAbilityByTagWithAutoApproach(FGameplayTag AbilityTag);

	// --- Floor Marker ---

	/** Spawn a floor marker at the given world location. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Movement")
	void SpawnFloorMarker(const FVector& WorldLocation);

	// --- Movement Target (for rotation) ---

	/** Set the movement target location for rotation purposes. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Movement")
	void SetMovementTarget(const FVector& TargetLocation);

	/** Clear the movement target. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Movement")
	void ClearMovementTarget();

	// --- Auto-Approach (Double-Click Attack) ---

	/**
	 * Queue an attack on the given target. If target is out of range, auto-approach first.
	 * Called by KDClickInputHandler on double-click.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combat")
	void QueueAttackOnApproach(AActor* Target);

	/**
	 * Cancel any queued attack (e.g., when clicking on ground to move).
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combat")
	void CancelQueuedAttack();

	// --- Auto-Repeat Basic Attack (Silkroad-style) ---

	/**
	 * Start auto-repeating basic attacks on the current target.
	 * Auto-repeat continues until:
	 * - Target is lost or dies
	 * - Player activates another skill
	 * - Player clicks ground to move
	 * - Insufficient resources (stamina)
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combat")
	void StartBasicAttackAutoRepeat();

	/**
	 * Stop auto-repeating basic attacks.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Combat")
	void StopBasicAttackAutoRepeat();

	/**
	 * Attempt to execute the next attack in auto-repeat chain.
	 * Called internally after basic attack montage completes.
	 */
	void TryAutoRepeatAttack();

	/** Check if auto-repeat is active. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Combat")
	bool IsAutoRepeatActive() const { return bAutoRepeatBasicAttack; }

	/** True when the character is auto-approaching a target to attack. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Combat")
	bool bWantsToAttack;

	/** True when auto-repeating basic attacks (Silkroad-style). */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Combat")
	bool bAutoRepeatBasicAttack;

	/** The target we're auto-approaching to attack. */
	UPROPERTY()
	TWeakObjectPtr<AActor> QueuedAttackTarget;

	/** Ability tag to activate when the queued auto-approach reaches valid range. */
	UPROPERTY()
	FGameplayTag QueuedAbilityTag;

	/** Acceptance radius for auto-approach (how close before we attack). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Combat")
	float ApproachAcceptanceRadius;

	// --- Team ---

	/** The combat team this character belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Combat")
	FGameplayTag CombatTeamTag;

	// --- Ability Event Handlers (for auto-repeat) ---

	/** Called when any ability ends. Drives auto-repeat for basic attacks. */
	UFUNCTION()
	void OnAbilityEnded(FGameplayTag AbilityTag);

	/** Called when any ability starts. Stops auto-repeat for non-basic skills. */
	UFUNCTION()
	void OnAbilityStarted(FGameplayTag AbilityTag);

	// --- Death ---

	/** Handle death event from damage component. */
	UFUNCTION()
	void HandleDeath();

	/** Is this character dead? */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Combat")
	bool IsDead() const;

	// --- Legacy Combat State (Class/Branch/Weapon) ---

	/** Get the character's combat class family. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|CombatState")
	EKDCombatClass GetCombatClass() const { return CombatClass; }

	/** Set the character's combat class family. Also syncs mastery identity. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|CombatState")
	void SetCombatClass(EKDCombatClass NewClass);

	/** Get the character's combat branch. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|CombatState")
	EKDCombatBranch GetCombatBranch() const { return CombatBranch; }

	/** Set the character's combat branch. Also syncs mastery identity. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|CombatState")
	void SetCombatBranch(EKDCombatBranch NewBranch);

	/** Get the character's primary equipped weapon type. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|CombatState")
	EKDWeaponType GetEquippedWeapon() const { return EquippedWeapon; }

	/** Set the character's primary equipped weapon type. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|CombatState")
	void SetEquippedWeapon(EKDWeaponType NewWeapon);

	/** Get the weapon attachment profile for a given weapon family. Returns empty profile if not found. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|CombatState")
	FKDWeaponAttachProfile GetWeaponAttachmentProfile(EKDWeaponFamily Family) const;

	/** Set or update a weapon attachment profile. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|CombatState")
	void SetWeaponAttachmentProfile(EKDWeaponFamily Family, const FKDWeaponAttachProfile& Profile);

	// --- Mastery Identity (PRIMARY) ---

	/**
	 * Get the character's mastery identity (class + branch pair).
	 * Preferred API for new mastery-system code.
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Mastery")
	FKDMasteryIdentity GetMasteryIdentity() const { return MasteryIdentity; }

	/**
	 * Set the character's mastery identity and sync legacy mirror fields.
	 * Preferred setter for new code.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Mastery")
	void SetMasteryIdentity(FKDMasteryIdentity NewIdentity);

	// --- IKDEntityInterface: Legacy Implementations ---
	virtual EKDCombatClass GetEntityCombatClass_Implementation() const override;
	virtual EKDCombatBranch GetEntityCombatBranch_Implementation() const override;
	virtual EKDWeaponType GetEntityWeaponType_Implementation() const override;

	// --- IKDEntityInterface: Mastery Implementation ---
	virtual FKDMasteryIdentity GetEntityMasteryIdentity_Implementation() const override;

	// --- Build System ---

	/**
	 * Build definition to apply on BeginPlay.
	 * When set, this build becomes the runtime source of truth for class/branch/weapon/abilities.
	 * When null, fallback to legacy test-mode branch/class setup.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kingdawn|Build")
	TObjectPtr<UKDBuildDefinition> SelectedBuildDefinition;

	/**
	 * The currently active build definition (transient runtime copy of SelectedBuildDefinition).
	 * This is what runtime systems query for build-driven configuration.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Kingdawn|Build")
	TObjectPtr<UKDBuildDefinition> ActiveBuildDefinition;

	/**
	 * Apply a build definition to this character.
	 * Sets class/branch/weapon state from the build and caches it for later reference.
	 * Does NOT grant abilities or apply visual state yet (Phase 3+).
	 *
	 * @param BuildDef The build definition to apply (can be null to clear)
	 * @return True if a build was successfully applied, false if BuildDef was null or invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Build")
	bool ApplyBuildDefinition(UKDBuildDefinition* BuildDef);

	/**
	 * Get the currently active build definition (may be null).
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Build")
	UKDBuildDefinition* GetActiveBuildDefinition() const { return ActiveBuildDefinition; }

	/**
	 * Check if a CVar test build override is active.
	 * Returns the override build definition if kd.build.testoverride > 0,
	 * or nullptr if no override is active.
	 * CVar values: 0=none, 1=BladeShield, 2=Wizard
	 */
	UKDBuildDefinition* GetTestBuildOverride() const;

	// --- Ability Granting ---

	/** Abilities to grant on BeginPlay (legacy fallback when no build is selected). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	/** Initialize GAS: create attribute set, grant abilities. */
	virtual void InitializeAbilitySystem();

	/** Rotate toward velocity or queued move target while moving. */
	void UpdateMovementFacing(float DeltaTime);

	/** Get the best current direction to face for movement. */
	FVector GetDesiredMovementFacingDirection() const;

	/** Update debug text showing active status effects (non-player enemies only). */
	void UpdateStatusDebugIndicator();

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDStatsComponent> StatsComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDDamageComponent> DamageComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDCollisionComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDTargetingComponent> TargetingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDFloorMarkerComponent> FloorMarkerComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDMasteryComponent> MasteryComp;

	/** Visual weapon mesh component. Used for static-mesh weapons like the Wizard staff. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UStaticMeshComponent> WeaponMeshComp;

	/** Main-hand visual weapon mesh component. Used for skeletal-mesh melee weapons like swords. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<USkeletalMeshComponent> MainHandWeaponMeshComp;

	/** Off-hand visual weapon mesh component. Used for skeletal-mesh shields. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<USkeletalMeshComponent> OffHandWeaponMeshComp;

	/** Debug-only status indicator shown above non-player enemies when a status tag is active. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UTextRenderComponent> StatusDebugTextComp;

	UPROPERTY()
	TObjectPtr<UKDCombatAttributeSet> CombatAttributes;

	/** Tag for the attack ability to trigger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Combat")
	FGameplayTag AttackAbilityTag;

	// --- Mastery Identity (PRIMARY) ---

	/**
	 * The character's mastery identity (class + branch pair).
	 * This is the primary authority for mastery-based identity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Mastery", meta = (AllowPrivateAccess = "true"))
	FKDMasteryIdentity MasteryIdentity;

	// --- Legacy Combat State (MIRROR) ---

	/**
	 * LEGACY MIRROR: The character's combat class family.
	 * Auto-populated from MasteryIdentity when SetMasteryIdentity() is called.
	 * Can still be set directly for legacy compatibility.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|CombatState", meta = (AllowPrivateAccess = "true"))
	EKDCombatClass CombatClass;

	/**
	 * LEGACY MIRROR: The character's combat branch.
	 * Auto-populated from MasteryIdentity when SetMasteryIdentity() is called.
	 * Can still be set directly for legacy compatibility.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|CombatState", meta = (AllowPrivateAccess = "true"))
	EKDCombatBranch CombatBranch;

	// --- Weapon Identity (INDEPENDENT) ---

	/**
	 * The character's primary equipped weapon type.
	 * Weapon identity is separate from mastery identity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|CombatState", meta = (AllowPrivateAccess = "true"))
	EKDWeaponType EquippedWeapon;

	/** Universal weapon family attachment profiles. One profile per weapon family. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|CombatState")
	TMap<EKDWeaponFamily, FKDWeaponAttachProfile> WeaponProfiles;


private:
	/** Update the visual weapon mesh based on EquippedWeapon. */
	void UpdateWeaponMesh();

	/**
	 * Get the weapon family for a given weapon type.
	 * Maps EKDWeaponType to EKDWeaponFamily for attachment purposes.
	 */
	static EKDWeaponFamily GetWeaponFamilyForType(EKDWeaponType WeaponType);

	/**
	 * Attach a weapon mesh component using the family profile system.
	 * Handles socket lookup, reference fallback, and transform application.
	 * Works for both USkeletalMeshComponent and UStaticMeshComponent.
	 */
	void AttachWeaponByProfile(USceneComponent* WeaponComp, USkeletalMeshComponent* CharMesh, const FKDWeaponAttachProfile& Profile);

	/**
	 * Load and apply the reference socket transform if the runtime mesh lacks the socket.
	 */
	bool TryApplyReferenceSocketTransform(USceneComponent* WeaponComp, const FKDWeaponAttachProfile& Profile);

	/**
	 * Apply a weapon visual profile to a skeletal mesh component.
	 * Loads the mesh asset from the profile and attaches it to the character.
	 * Used by build-driven weapon visual setup.
	 */
	bool ApplyWeaponVisualProfile(USkeletalMeshComponent* WeaponComp, USkeletalMeshComponent* CharMesh, const UKDWeaponVisualProfile* Profile);

	bool bAbilitySystemInitialized;

	/** The target location we're moving toward (for rotation). */
	UPROPERTY()
	FVector MovementTargetLocation;

	/** Whether we have a valid movement target. */
	UPROPERTY()
	bool bHasMovementTarget;

	/** Interp speed for manual movement-facing rotation. */
	UPROPERTY(EditDefaultsOnly, Category = "Kingdawn|Movement")
	float MovementFacingInterpSpeed;

	/** Turn rate in degrees/sec for movement-facing. */
	UPROPERTY(EditDefaultsOnly, Category = "Kingdawn|Movement")
	float MovementFacingTurnRate;

	/** Minimum 2D speed before movement-facing turns the pawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Kingdawn|Movement")
	float MovementFacingMinSpeed;

	/** Distance threshold for clearing a point-move target. */
	UPROPERTY(EditDefaultsOnly, Category = "Kingdawn|Movement")
	float MovementTargetReachedThreshold;

	/** If true, apply a default yaw offset to inherited mannequin-style meshes that are still at 0 yaw. */
	UPROPERTY(EditDefaultsOnly, Category = "Kingdawn|Movement")
	bool bAutoAlignMeshYaw;

	/** Standard mesh yaw offset used to align mannequin meshes to actor forward. */
	UPROPERTY(EditDefaultsOnly, Category = "Kingdawn|Movement")
	float MeshYawOffset;
};



