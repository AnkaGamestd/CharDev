// Copyright Kingdawn. All Rights Reserved.
// Attack ability - plays montage, consumes stamina, enables collision during notify windows

#pragma once

#include "Abilities/KDGameplayAbility.h"
#include "CoreMinimal.h"
#include "KDGA_Attack.generated.h"

/**
 * Concrete attack ability.
 * Plays an attack montage and enables weapon collision during the notify window.
 */
UCLASS()
class KINGDAWNCOMBAT_API UKDGA_Attack : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGA_Attack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
