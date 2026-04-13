// Copyright Kingdawn. All Rights Reserved.

#include "AI/KDEnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

AKDEnemyController::AKDEnemyController()
{
	// Blackboard component is auto-created by AAIController base class
	// Perception component is optional — not used in phase 1 (BTService handles target detection)
}

void AKDEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	// Run the assigned BehaviorTree (if set in Blueprint or DefaultBehaviorTree)
	if (DefaultBehaviorTree)
	{
		RunBehaviorTree(DefaultBehaviorTree);
		UE_LOG(LogTemp, Log, TEXT("KDEnemyController: Started BehaviorTree %s for pawn %s"),
			*GetNameSafe(DefaultBehaviorTree), *InPawn->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDEnemyController: No BehaviorTree assigned for pawn %s — AI will be idle"),
			*InPawn->GetName());
	}
}

void AKDEnemyController::OnUnPossess()
{
	// Stop the BehaviorTree if running (e.g., on death)
	if (BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("Unpossessed"));
	}

	Super::OnUnPossess();
}
