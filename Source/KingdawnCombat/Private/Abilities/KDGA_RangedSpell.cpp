// Copyright Kingdawn. All Rights Reserved.

#include "Abilities/KDGA_RangedSpell.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/KDStatsComponent.h"
#include "Components/KDTargetingComponent.h"
#include "Components/KDDamageComponent.h"
#include "Core/KDGameplayTags.h"
#include "GameFramework/Character.h"

UKDGA_RangedSpell::UKDGA_RangedSpell()
{
	// Wizard spell defaults - override base melee defaults
	BaseDamage = 25.f;
	ProjectileSpeed = 0.f; // instant by default
	bUsesProjectile = false;
	ProjectileSpawnSocket = FName("Muzzle_01");
	HitEffect = nullptr;

	// Override base ability defaults for ranged spells
	ManaCost = 15.f;
	StaminaCost = 0.f;
	bRequiresTarget = true;
	MinRange = 0.f;
	MaxRange = 1500.f;
	bAutoFaceTarget = true;
	FacingInterpSpeed = 0.f; // instant snap to target
}

void UKDGA_RangedSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!AbilityMontage)
	{
		FireSpell();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Listen for gameplay event from montage notify to fire the spell.
	// The montage should have a "KD.Ability.Event.Fire" anim notify at the cast point.
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(FName("KD.Ability.Event.Fire")),
		nullptr,
		false,
		false
	);

	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UKDGA_RangedSpell::OnSpellEventReceived);
		WaitEventTask->ReadyForActivation();
	}
	else
	{
		// Fallback: if no event task could be created, fire immediately
		UE_LOG(LogTemp, Warning, TEXT("KDGA_RangedSpell: Could not create WaitGameplayEvent task for %s, firing immediately"), *GetName());
		FireSpell();
	}
}

void UKDGA_RangedSpell::OnSpellEventReceived(FGameplayEventData Payload)
{
	FireSpell();
}

void UKDGA_RangedSpell::FireSpell_Implementation()
{
	// Default: instant-hit ranged damage to current target
	AActor* Target = GetAbilityTarget();
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDGA_RangedSpell::FireSpell - No target for %s"), *GetName());
		return;
	}

	// Apply damage through KDDamageComponent pipeline (formula + events + immortality guard)
	AActor* DamageCauser = CurrentActorInfo && CurrentActorInfo->OwnerActor.IsValid() ? CurrentActorInfo->OwnerActor.Get() : nullptr;
	ApplySpellDamageToTarget(Target, BaseDamage, DamageCauser);

	// Apply hit effect if configured
	if (HitEffect && CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* TargetASC = Target->FindComponentByClass<UAbilitySystemComponent>();
		if (TargetASC)
		{
			FGameplayEffectSpecHandle SpecHandle = CurrentActorInfo->AbilitySystemComponent->MakeOutgoingSpec(HitEffect, 1, CurrentActorInfo->AbilitySystemComponent->MakeEffectContext());
			if (SpecHandle.IsValid())
			{
				CurrentActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

void UKDGA_RangedSpell::ApplySpellDamageToTarget(AActor* Target, float RawDamage, AActor* DamageCauser)
{
	if (!Target || RawDamage <= 0.f)
	{
		return;
	}

	// Route through KDDamageComponent pipeline — ensures immortality guard,
	// LastDamageReceived, OnDamageReceived, OnDamageInflicted all fire correctly.
	UKDDamageComponent* TargetDamageComp = Target->FindComponentByClass<UKDDamageComponent>();
	if (TargetDamageComp)
	{
		TargetDamageComp->ApplyAbilityDamage(RawDamage, DamageCauser);
	}
	else
	{
		// No damage component — damage is NOT applied.
		// Controlled degradation: rather than bypass the event pipeline
		// (immortality, delegates, LastDamageReceived), we refuse to apply damage
		// to actors that cannot participate in the full damage system.
		// All combat actors MUST have KDDamageComponent; this path only triggers
		// for misconfigured or non-combat actors.
		UE_LOG(LogTemp, Warning, TEXT("KDGA_RangedSpell: Target '%s' has no KDDamageComponent — damage NOT applied (%.1f raw). "
			"Add KDDamageComponent to this actor to receive spell damage."),
			*Target->GetName(), RawDamage);
	}
}
