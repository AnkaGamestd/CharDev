// Copyright Kingdawn. All Rights Reserved.

#include "AI/BTDecorator_IsInRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsInRange::UBTDecorator_IsInRange()
{
	NodeName = "Is In Attack Range";
	AttackRange = 110.f; // Default melee attack range, matches KDGA_Attack::MaxRange
}

bool UBTDecorator_IsInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* OwnerPawn = AIController->GetPawn();
	if (!OwnerPawn)
	{
		return false;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	const FName TargetActorKeyName = TEXT("TargetActor");
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKeyName));
	if (!TargetActor)
	{
		return false;
	}

	const float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());
	return Distance <= AttackRange;
}
