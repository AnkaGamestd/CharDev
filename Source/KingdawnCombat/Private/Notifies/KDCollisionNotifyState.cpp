// Copyright Kingdawn. All Rights Reserved.

#include "Notifies/KDCollisionNotifyState.h"
#include "Components/KDCollisionComponent.h"
#include "GameFramework/Character.h"

void UKDCollisionNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	UKDCollisionComponent* CollisionComp = MeshComp->GetOwner()->FindComponentByClass<UKDCollisionComponent>();
	if (CollisionComp)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("KDCombat: Collision window OPEN on %s"), *MeshComp->GetOwner()->GetName());
		CollisionComp->StartAllTraces();
	}
}

void UKDCollisionNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	UKDCollisionComponent* CollisionComp = MeshComp->GetOwner()->FindComponentByClass<UKDCollisionComponent>();
	if (CollisionComp)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("KDCombat: Collision window CLOSED on %s"), *MeshComp->GetOwner()->GetName());
		CollisionComp->StopAllTraces();
	}
}
