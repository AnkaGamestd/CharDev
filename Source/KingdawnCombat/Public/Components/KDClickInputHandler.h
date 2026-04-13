// Copyright Kingdawn. All Rights Reserved.
// Click input handler for Silkroad-style click-to-move/click-to-target.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KDClickInputHandler.generated.h"

class AKDCombatCharacter;

/**
 * Handles left-click disambiguation for Silkroad-style controls:
 * - Single click on enemy = select target
 * - Double click on enemy = select target + auto-approach + attack
 * - Single click on ground = click-to-move
 * Attached to KDPlayerController.
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDClickInputHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDClickInputHandler();

	// --- Input Callbacks ---

	/** Called when left mouse button is pressed. */
	void OnLeftClickPressed();

	/** Called when left mouse button is released. */
	void OnLeftClickReleased();

	// --- Configuration ---

	/** Time window for double-click detection (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Input")
	float DoubleClickThreshold;

	/** Trace channel for click raycast (e.g., ECC_Visibility or ECC_Pawn). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Input")
	TEnumAsByte<ECollisionChannel> ClickTraceChannel;

	/** Maximum distance for click raycast. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Input")
	float MaxClickTraceDistance;

	/** Get the possessed combat character (caches result, auto-invalidates on pawn change). */
	AKDCombatCharacter* GetPossessedCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Pending double-click state. */
	bool bPendingDoubleClick;

	/** Time of the last click. */
	float LastClickTime;

	/** The actor clicked on in the last click. */
	UPROPERTY()
	TWeakObjectPtr<AActor> LastClickedActor;

	/** Cached hit location from the last click raycast. */
	FVector LastClickHitLocation;

	/** Cached owner controller. */
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerController;

	/** Cached possessed combat character. */
	UPROPERTY()
	TWeakObjectPtr<AKDCombatCharacter> PossessedCharacter;

	/** Process a single left-click. */
	void ProcessSingleClick();

	/** Process a double left-click. */
	void ProcessDoubleClick();

	/** Perform a raycast from the cursor to the world. Returns hit actor or nullptr. */
	AActor* PerformClickRaycast(FVector& OutHitLocation);

	/** Handle a click on an enemy actor. */
	void HandleClickOnEnemy(AActor* Enemy, bool bIsDoubleClick);

	/** Handle a click on the ground. */
	void HandleClickOnGround(const FVector& GroundLocation);

	/** Check if an actor is a valid hostile target for the possessed character. */
	bool IsValidHostileForCharacter(AActor* Actor) const;
};
