// Copyright Kingdawn. All Rights Reserved.
// Targeting component for MMO-style tab-target combat.
// Manages target selection, persistence, validation, and cycling.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KDTargetingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKDOnTargetChanged, AActor*, OldTarget, AActor*, NewTarget);

/**
 * Manages target selection for MMO-style combat.
 * Supports tab-cycling, click selection, target validation, and range checks.
 * No dodge, no iframe — pure target-based combat.
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDTargetingComponent();

	// --- Delegates ---

	/** Broadcast when the selected target changes. */
	UPROPERTY(BlueprintAssignable, Category = "Kingdawn|Targeting")
	FKDOnTargetChanged OnTargetChanged;

	// --- Target Selection ---

	/** Set the current target. Pass nullptr to clear. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Targeting")
	void SetTarget(AActor* NewTarget);

	/** Clear the current target. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Targeting")
	void ClearTarget();

	/** Get the current target (may be null or invalid). */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	AActor* GetCurrentTarget() const;

	/** Get the current target, with validity check. Returns null if invalid. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	AActor* GetValidTarget() const;

	// --- Validation ---

	/** Is the given actor a valid hostile target for this character? */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	bool IsValidHostileTarget(AActor* Target) const;

	/** Is the current target still valid (alive, hostile, in range)? */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	bool IsCurrentTargetValid() const;

	/** Is the current target within the given range? */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	bool IsTargetInRange(float Range) const;

	/** Get distance to current target. Returns -1 if no target. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Targeting")
	float GetTargetDistance() const;

	// --- Target Cycling ---

	/**
	 * Cycle to the next nearest hostile target.
	 * If bForward is true, cycles forward through sorted list; false cycles backward.
	 * If no current target, selects the nearest hostile.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Targeting")
	void CycleTarget(bool bForward = true);

	/** Select the nearest hostile target within MaxCycleDistance. */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Targeting")
	void SelectNearestTarget();

	// --- Configuration ---

	/** Maximum distance for target retention. Target beyond this is auto-cleared. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Targeting")
	float MaxTargetDistance;

	/** Maximum distance for tab-cycling to find targets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Targeting")
	float MaxCycleDistance;

	/** How often to validate the current target (seconds). 0 = every frame. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Targeting")
	float ValidationInterval;

	/** Collision channel used for line-of-sight checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Targeting")
	TEnumAsByte<ECollisionChannel> LineOfSightChannel;

	/** If true, draw a debug circle around the current target. For testing only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Targeting|Debug")
	bool bShowDebugTargetIndicator;

	/** Color of the debug target indicator ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Targeting|Debug")
	FLinearColor DebugTargetColor;

	/** Radius of the debug target indicator ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Targeting|Debug")
	float DebugTargetRingRadius;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** The currently selected target. */
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;

	/** Time since last validation check. */
	float TimeSinceValidation;

	/** Internal: validate and potentially clear current target. */
	void ValidateCurrentTarget();

	/** Internal: find all hostile actors within range, sorted by distance. */
	TArray<AActor*> FindHostileTargetsInRange(float Range) const;

	/** Internal: check if an actor is alive. */
	bool IsActorAlive(AActor* Actor) const;
};
