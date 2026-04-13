// Copyright Kingdawn. All Rights Reserved.

#include "AI/BTTask_SetCombatTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/KDCombatCharacter.h"

UBTTask_SetCombatTarget::UBTTask_SetCombatTarget()
{
	NodeName = "Set Combat Target";
}

EBTNodeResult::Type UBTTask_SetCombatTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	const FName TargetActorKeyName = TEXT("TargetActor");
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKeyName));

	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	// Reuse the existing character API to set the combat target.
	// This also triggers OnTargetChanged delegate, debug ring, etc.
	OwnerChar->SetCombatTarget(TargetActor);

	return EBTNodeResult::Succeeded;
}
