// Copyright Kingdawn. All Rights Reserved.
// Adapted from CollisionsManager/ACMCollisionManagerComponent.h
// Removed: ACFAnimInstance dependencies, ACF-specific subsystems

#pragma once

#include "Collision/KDCollisionTypes.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KDCollisionComponent.generated.h"

class AActor;
class UDamageType;
class UMeshComponent;

/**
 * Delegate triggered when a collision is detected.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKDCollisionDetected, const FHitResult&, HitResult);

/**
 * Delegate triggered when an actor receives damage.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKDActorDamaged, AActor*, DamageReceiver);

/**
 * Manages collision detection and damage handling for weapon attacks.
 * Supports bone-based sphere sweeps, area damage, and swipe traces.
 * No animation system dependencies - triggered by notifies or direct calls.
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDCollisionComponent();

	// --- Configuration ---

	/** Debug display mode. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn|Debug")
	EKDDDebugType ShowDebugInfo;

	/** Color for inactive debug traces. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn|Debug")
	FLinearColor DebugInactiveColor;

	/** Color for active debug traces. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn|Debug")
	FLinearColor DebugActiveColor;

	/** If true, each swing can hit multiple actors. If false, each actor is hit once per swing. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn")
	bool bAllowMultipleHitsPerSwing;

	/** Collision channels to trace against. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn")
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels;

	/** Actors to ignore during collision detection. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn")
	TArray<TObjectPtr<AActor>> IgnoredActors;

	/** If true, the component owner is ignored during traces. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn")
	bool bIgnoreOwner;

	/** If true, automatically calls ApplyDamage on hit. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn")
	bool bAutoApplyDamage;

	/** Named trace configurations (e.g., "SwordSlash", "AxeChop"). */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn|Traces")
	TMap<FName, FKDTraceInfo> DamageTraces;

	/** Swipe trace configuration (single-shot trace between two points). */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn|Traces")
	FKDBaseTraceInfo SwipeTraceInfo;

	/** Area damage trace configuration. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn|Traces")
	FKDBaseTraceInfo AreaDamageTraceInfo;

	// --- Delegates ---

	/** Broadcast when a collision is detected. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDCollisionDetected OnCollisionDetected;

	/** Broadcast when an actor takes damage. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn")
	FKDActorDamaged OnActorDamaged;

	// --- Setup ---

	/**
	 * Initialize the collision manager with a damage mesh.
	 * @param InDamageMesh The mesh component containing the trace sockets.
	 * @param DefaultChannels Default collision channels to trace.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn", meta = (AutoCreateRefTerm = "defaultChannels"))
	void SetupCollisionManager(UMeshComponent* InDamageMesh,
		const TArray<TEnumAsByte<ECollisionChannel>>& DefaultChannels);

	/**
	 * Set the actor owner for damage purposes.
	 * Useful when the component is on a weapon but damage should be attributed to the character.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void SetActorOwner(AActor* NewOwner);

	/** Get the current actor owner. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	AActor* GetActorOwner() const;

	// --- Trace Control ---

	/** Start all configured traces. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StartAllTraces();

	/** Stop all active traces. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StopAllTraces();

	/** Start a single named trace. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StartSingleTrace(const FName& Name);

	/** Stop a single named trace. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StopSingleTrace(const FName& Name);

	/** Start a single trace for a duration. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StartTimedSingleTrace(const FName& TraceName, float Duration);

	/** Start all traces for a duration. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StartAllTimedTraces(float Duration);

	/** Check if a trace is currently active. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kingdawn")
	bool IsTraceActive(const FName& TraceName) const;

	// --- Single-Shot Traces ---

	/**
	 * Perform a single swipe trace between two points.
	 * @param Start Start point of the trace.
	 * @param End End point of the trace.
	 * @param Radius Sphere sweep radius.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void PerformSwipeTraceShot(const FVector& Start, const FVector& End, float Radius = 0.f);

	/**
	 * Perform a single swipe trace locally (returns hit result).
	 * @return True if hit occurred.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	bool PerformSwipeTraceShot_Local(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit);

	// --- Area Damage ---

	/**
	 * Start continuous area damage at a location.
	 * @param DamageCenter Center of the damage area.
	 * @param DamageRadius Radius of the damage area.
	 * @param DamageInterval Time between damage applications.
	 * @param DamageTypeOverride Optional damage type override.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StartAreaDamage(const FVector& DamageCenter, float DamageRadius, float DamageInterval = 1.f,
		TSubclassOf<UDamageType> DamageTypeOverride = nullptr);

	/** Stop current area damage. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void StopCurrentAreaDamage();

	/**
	 * Perform a single area damage pulse.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void PerformAreaDamage_Single(const FVector& DamageCenter, float DamageRadius,
		TSubclassOf<UDamageType> DamageTypeOverride = nullptr);

	/**
	 * Perform area damage for a duration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void PerformAreaDamageForDuration(const FVector& DamageCenter, float DamageRadius, float Duration, float DamageInterval = 1.f);

	// --- Configuration ---

	/** Add an actor to the ignore list. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void AddActorToIgnore(AActor* IgnoredActor);

	/** Add a collision channel to trace. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void AddCollisionChannel(TEnumAsByte<ECollisionChannel> InTraceChannel);

	/** Add multiple collision channels. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void AddCollisionChannels(TArray<TEnumAsByte<ECollisionChannel>> InTraceChannels);

	/** Clear all collision channels. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void ClearCollisionChannels();

	/** Set trace configuration. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	void SetTraceConfig(const FName& TraceName, const FKDTraceInfo& TraceInfo);

	/** Get trace configuration. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	TMap<FName, FKDTraceInfo> GetDamageTraces() const { return DamageTraces; }

	/** Try to get a specific trace config. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn")
	bool TryGetTraceConfig(const FName& TraceName, FKDTraceInfo& OutTraceInfo) const
	{
		if (DamageTraces.Contains(TraceName))
		{
			OutTraceInfo = *DamageTraces.Find(TraceName);
			return true;
		}
		return false;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// --- Internal State ---

	UPROPERTY()
	TObjectPtr<AActor> ActorOwner;

	UPROPERTY()
	TObjectPtr<UMeshComponent> DamageMesh;

	UPROPERTY()
	TMap<FName, FKDTraceInfo> ActivatedTraces;

	UPROPERTY()
	TArray<FName> PendingDelete;

	UPROPERTY()
	TMap<FName, FKDHitActors> AlreadyHitActors;

	UPROPERTY()
	TMap<FName, FTimerHandle> TraceTimers;

	UPROPERTY()
	TMap<FName, TObjectPtr<UNiagaraComponent>> NiagaraSystemComponents;

	UPROPERTY()
	FTimerHandle AllTraceTimer;

	UPROPERTY()
	FTimerHandle AreaDamageTimer;

	UPROPERTY()
	FKDAreaDamageInfo CurrentAreaDamage;

	UPROPERTY()
	bool bIsStarted;

	UPROPERTY()
	bool bSingleTimedTraceStarted;

	UPROPERTY()
	bool bAllTimedTraceStarted;

	// --- Internal Methods ---

	void UpdateCollisions();
	void SetStarted(bool InStarted);
	FRotator GetLineRotation(FVector Start, FVector End) const;
	void DisplayDebugTraces();
	void ShowDebugTrace(const FVector& StartPos, const FVector& EndPos, float Radius, float Duration, FLinearColor Color = FLinearColor::Red);

	void ApplyDamage(const FHitResult& HitResult, const FKDBaseTraceInfo& CurrentTrace);
	void ApplyPointDamage(const FHitResult& HitResult, const FKDBaseTraceInfo& CurrentTrace);
	void ApplyAreaDamage(const FHitResult& HitResult, const FKDBaseTraceInfo& CurrentTrace);

	bool CanActorDamageActor(const AActor* Attacker, const AActor* Victim) const;

	void PlayTrails(const FName& Trail);
	void StopTrails(const FName& Trail);

	UFUNCTION()
	void HandleTimedSingleTraceFinished(const FName& TraceEnded);

	UFUNCTION()
	void HandleAllTimedTraceFinished();

	UFUNCTION()
	void HandleAreaDamageFinished();

	UFUNCTION()
	void HandleAreaDamageLooping();
};
