// Copyright Kingdawn. All Rights Reserved.
// MMO-style attack ability: requires target, range-gated, auto-facing.

#include "Abilities/KDGA_Attack.h"
#include "Core/KDGameplayTags.h"

UKDGA_Attack::UKDGA_Attack()
{
	// MMO targeting configuration for basic attack (mouse double-click)
	AbilityTag = FKDGameplayTags::Combat_Action_Attack_Basic;
	StaminaCost = 20.f;
	bCommitOnActivate = true;
	bEndOnMontageComplete = true;
	MontagePlayRate = 1.f;

	// Target requirement (MMO-style)
	bRequiresTarget = true;
	MinRange = 0.f;
	MaxRange = 110.f; // Fallback melee trace reach
	bAutoFaceTarget = true; // Auto-face target on attack
	FacingInterpSpeed = 0.f; // Instant facing (can be increased for smooth rotation)

	// Montage will be assigned in the Blueprint or via data asset
}

void UKDGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Call parent to handle target validation, facing, montage playback, and commit
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Additional attack-specific logic can go here (e.g., send gameplay event)
}
