// Copyright Kingdawn. All Rights Reserved.
// BTTask that sets the combat target from the Blackboard TargetActor key.
// Reuses AKDCombatCharacter::SetCombatTarget().

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "BTTask_SetCombatTarget.generated.h"

/**
 * Reads TargetActor from the Blackboard and sets it as the character's combat target
 * via AKDCombatCharacter::SetCombatTarget().
 *
 * Blackboard Keys (read):
 * - TargetActor (Object): The actor to set as the combat target.
 */
UCLASS()
class KINGDAWNCOMBAT_API UBTTask_SetCombatTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetCombatTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
