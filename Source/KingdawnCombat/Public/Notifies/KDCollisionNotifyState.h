// Copyright Kingdawn. All Rights Reserved.
// Animation notify state to enable/disable collision during attack montages

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CoreMinimal.h"
#include "KDCollisionNotifyState.generated.h"

/**
 * AnimNotifyState that enables collision on begin and disables on end.
 * Used in attack montages to control weapon collision windows.
 */
UCLASS()
class KINGDAWNCOMBAT_API UKDCollisionNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** Optional collision channel name to enable/disable. Leave empty to toggle all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn")
	FName CollisionChannelName;
};
