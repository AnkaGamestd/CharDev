// Copyright Kingdawn. All Rights Reserved.

#include "Abilities/KDGA_ManaShield.h"
#include "AbilitySystemComponent.h"
#include "Components/KDStatsComponent.h"
#include "Core/KDGameplayTags.h"

UKDGA_ManaShield::UKDGA_ManaShield()
{
	// ManaShield defaults
	ShieldEffect = nullptr;
	ShieldDuration = 10.f;
	AbsorptionRatio = 0.5f; // 50% of damage absorbed by mana
	MaxAbsorption = 0.f;    // unlimited absorption, mana-gated

	// Override base ability defaults
	ManaCost = 30.f;
	StaminaCost = 0.f;
	bRequiresTarget = false; // Self-cast
	bAutoFaceTarget = false;
	MaxRange = 0.f;
	bEndOnMontageComplete = true;
}

void UKDGA_ManaShield::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Let base handle: commit, mana consumption, montage
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Apply the shield buff
	ApplyShieldBuff();

	// Notify Blueprint for VFX
	BP_OnShieldActivated();
}

void UKDGA_ManaShield::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove the shield buff when ability ends
	RemoveShieldBuff();

	BP_OnShieldDeactivated();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKDGA_ManaShield::ApplyShieldBuff()
{
	if (!ShieldEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDGA_ManaShield: No ShieldEffect GE assigned for %s"), *GetName());
		return;
	}

	if (!CurrentActorInfo || !CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
		ShieldEffect, 1, ASC->MakeEffectContext());

	if (!SpecHandle.IsValid())
	{
		return;
	}

	// Set duration via SetByCaller
	if (ShieldDuration > 0.f)
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(
			FKDGameplayTags::Data_Duration, ShieldDuration);
	}

	// Set absorption ratio
	SpecHandle.Data.Get()->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("KD.Data.AbsorptionRatio")), AbsorptionRatio);

	// Set max absorption if capped
	if (MaxAbsorption > 0.f)
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(FName("KD.Data.MaxAbsorption")), MaxAbsorption);
	}

	// Apply to self
	ShieldEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	UE_LOG(LogTemp, Log, TEXT("KDGA_ManaShield: Applied shield buff (duration %.1f, absorption %.0f%%)"),
		ShieldDuration, AbsorptionRatio * 100.f);
}

void UKDGA_ManaShield::RemoveShieldBuff()
{
	if (ShieldEffectHandle.IsValid() && CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		ASC->RemoveActiveGameplayEffect(ShieldEffectHandle);

		UE_LOG(LogTemp, Log, TEXT("KDGA_ManaShield: Removed shield buff"));
	}

	ShieldEffectHandle = FActiveGameplayEffectHandle();
}
