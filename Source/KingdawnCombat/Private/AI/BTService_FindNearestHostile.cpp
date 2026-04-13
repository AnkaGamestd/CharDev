// Copyright Kingdawn. All Rights Reserved.

#include "AI/BTService_FindNearestHostile.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/KDCombatCharacter.h"
#include "Components/KDTargetingComponent.h"

UBTService_FindNearestHostile::UBTService_FindNearestHostile()
{
	NodeName = "Find Nearest Hostile";
	Interval = 0.5f; // Default: check twice per second
	RandomDeviation = 0.1f;
	AggroRange = 0.f; // 0 = use targeting component's default MaxCycleDistance
}

void UBTService_FindNearestHostile::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	AKDCombatCharacter* OwnerChar = Cast<AKDCombatCharacter>(AIController->GetPawn());
	if (!OwnerChar)
	{
		return;
	}

	UKDTargetingComponent* TargetingComp = OwnerChar->FindComponentByClass<UKDTargetingComponent>();
	if (!TargetingComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTService_FindNearestHostile: No KDTargetingComponent on %s"), *OwnerChar->GetName());
		return;
	}

	// Temporarily override MaxCycleDistance if AggroRange is set
	const float OriginalMaxCycleDistance = TargetingComp->MaxCycleDistance;
	if (AggroRange > 0.f)
	{
		TargetingComp->MaxCycleDistance = AggroRange;
	}

	// Use the existing targeting component logic to find the nearest hostile
	TargetingComp->SelectNearestTarget();

	// Restore original MaxCycleDistance
	if (AggroRange > 0.f)
	{
		TargetingComp->MaxCycleDistance = OriginalMaxCycleDistance;
	}

	// Get the selected target from the targeting component
	AActor* FoundTarget = TargetingComp->GetValidTarget();

	// Write to Blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		// Find the TargetActor key (first Object key we encounter, or you can specify key name)
		const FName TargetActorKeyName = TEXT("TargetActor");
		const FBlackboard::FKey TargetActorKey = BlackboardComp->GetKeyID(TargetActorKeyName);

		if (TargetActorKey != FBlackboard::InvalidKey)
		{
			if (FoundTarget)
			{
				BlackboardComp->SetValueAsObject(TargetActorKeyName, FoundTarget);
			}
			else
			{
				BlackboardComp->ClearValue(TargetActorKeyName);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BTService_FindNearestHostile: Blackboard key 'TargetActor' not found"));
		}
	}
}
