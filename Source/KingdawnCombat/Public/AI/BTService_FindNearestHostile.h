// Copyright Kingdawn. All Rights Reserved.
// BTService that periodically finds the nearest hostile target and writes it to the Blackboard.
// Reuses UKDTargetingComponent::SelectNearestTarget() for hostile detection.

#pragma once

#include "BehaviorTree/BTService.h"
#include "CoreMinimal.h"
#include "BTService_FindNearestHostile.generated.h"

/**
 * Periodically scans for the nearest hostile target using the owning character's
 * KDTargetingComponent and writes the result to the Blackboard.
 *
 * Blackboard Keys:
 * - TargetActor (Object): Set to the nearest hostile actor, or cleared if none found.
 *
 * Configuration:
 * - AggroRange: Maximum detection range for hostile targets.
 *   Defaults to MaxCycleDistance from the targeting component if 0.
 */
UCLASS()
class KINGDAWNCOMBAT_API UBTService_FindNearestHostile : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FindNearestHostile();

	/** Maximum range to scan for hostile targets. 0 = use targeting component's MaxCycleDistance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|AI")
	float AggroRange;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
