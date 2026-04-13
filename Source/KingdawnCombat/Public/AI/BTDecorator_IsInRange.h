// Copyright Kingdawn. All Rights Reserved.
// BTDecorator that checks if the Blackboard target is within attack range.

#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "CoreMinimal.h"
#include "BTDecorator_IsInRange.generated.h"

/**
 * Checks if the TargetActor in the Blackboard is within AttackRange
 * of the owning character.
 *
 * Blackboard Keys (read):
 * - TargetActor (Object): The actor to check distance to.
 *
 * Configuration:
 * - AttackRange: Maximum distance considered "in range" for an attack.
 */
UCLASS()
class KINGDAWNCOMBAT_API UBTDecorator_IsInRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsInRange();

	/** Maximum distance to consider the target "in range". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|AI")
	float AttackRange;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
