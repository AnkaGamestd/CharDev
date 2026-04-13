// Copyright Kingdawn. All Rights Reserved.
// BTTask that triggers the character's basic attack via AKDCombatCharacter::TryAttack().

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "BTTask_TryAttack.generated.h"

/**
 * Calls AKDCombatCharacter::TryAttack() to trigger the basic attack ability.
 *
 * The character must have a valid combat target set (via SetCombatTarget).
 * TryAttack() internally:
 * - Validates the target exists, is in range, and is alive.
 * - Triggers the basic attack ability via the GAS ability system.
 * - Starts the auto-repeat attack loop if configured.
 *
 * Returns Success if the attack was successfully triggered, Failed otherwise.
 */
UCLASS()
class KINGDAWNCOMBAT_API UBTTask_TryAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TryAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
