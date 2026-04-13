// Copyright Kingdawn. All Rights Reserved.
// AIController for enemy NPCs. Runs a BehaviorTree assigned via DefaultBehaviorTree
// or set in a Blueprint subclass. Lightweight — no custom logic beyond BT startup.

#pragma once

#include "AIController.h"
#include "CoreMinimal.h"
#include "KDEnemyController.generated.h"

class UBehaviorTree;
class UBlackboardData;

/**
 * Base AIController for Kingdawn enemy NPCs.
 *
 * Responsibilities:
 * - Possess the enemy pawn and start the assigned BehaviorTree.
 * - Provide the Blackboard component (auto-created by AAIController).
 *
 * Does NOT own combat state — all combat logic lives in AKDCombatCharacter
 * and its components. The BT tasks/services/decorators bridge the gap.
 */
UCLASS()
class KINGDAWNCOMBAT_API AKDEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AKDEnemyController();

	/** The BehaviorTree to run when this controller possesses a pawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Kingdawn|AI")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
};
