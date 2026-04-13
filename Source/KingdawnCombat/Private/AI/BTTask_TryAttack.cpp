// Copyright Kingdawn. All Rights Reserved.

#include "AI/BTTask_TryAttack.h"
#include "AIController.h"
#include "Characters/KDCombatCharacter.h"

UBTTask_TryAttack::UBTTask_TryAttack()
{
	NodeName = "Try Attack";
	bNotifyTick = false; // Instant execution, no tick needed
}

EBTNodeResult::Type UBTTask_TryAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AKDCombatCharacter* OwnerChar = Cast<AKDCombatCharacter>(AIController->GetPawn());
	if (!OwnerChar)
	{
		return EBTNodeResult::Failed;
	}

	// Call the existing TryAttack() method.
	// It handles:
	// - Target validation (alive, in range)
	// - Stamina check
	// - Ability activation via GAS
	// - Auto-repeat setup (if configured)
	const bool bSuccess = OwnerChar->TryAttack();

	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
