// Copyright Kingdawn. All Rights Reserved.

#include "Abilities/KDGA_ComboAttack.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/KDAbilitySystemComponent.h"
#include "Core/KDGameplayTags.h"

UKDGA_ComboAttack::UKDGA_ComboAttack()
{
	AbilityTag = FKDGameplayTags::Combat_Skill_Combo_Light;
	StaminaCost = 15.f;
	bCommitOnActivate = true;
	MontagePlayRate = 1.f;
	bEndOnMontageComplete = true;

	bRequiresTarget = true;
	MinRange = 0.f;
	MaxRange = 110.f;
	bAutoFaceTarget = true;
	FacingInterpSpeed = 0.f;

	bNextSectionQueued = false;
	CurrentSectionIndex = 0;
}

int32 UKDGA_ComboAttack::GetComboCounter() const
{
	UKDAbilitySystemComponent* KDASC = GetKDAbilitySystemComponent();
	if (KDASC)
	{
		return KDASC->GetComboCount(AbilityTag);
	}
	return 0;
}

void UKDGA_ComboAttack::ResetComboCounter()
{
	UKDAbilitySystemComponent* KDASC = GetKDAbilitySystemComponent();
	if (KDASC)
	{
		KDASC->ResetComboCount(AbilityTag);
	}
}

void UKDGA_ComboAttack::ForceComboCounter(int32 Value)
{
	UKDAbilitySystemComponent* KDASC = GetKDAbilitySystemComponent();
	if (KDASC)
	{
		KDASC->SetComboCounter(AbilityTag, Value);
	}
}

void UKDGA_ComboAttack::SendComboInput()
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("KDGA_ComboAttack: SendComboInput ignored - full combo autoplays on one activation"));
}

FName UKDGA_ComboAttack::GetMontageSectionName_Implementation()
{
	if (!AbilityMontage || AbilityMontage->CompositeSections.Num() == 0)
	{
		return NAME_None;
	}

	return AbilityMontage->GetSectionName(0);
}

void UKDGA_ComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	CurrentSectionIndex = 0;
	bNextSectionQueued = false;
	ResetComboCounter();

	const FName FirstSectionName = GetMontageSectionName();

	UE_LOG(LogTemp, Log, TEXT("KDGA_ComboAttack: ActivateAbility - Full combo start, FirstSection=%s"),
		*FirstSectionName.ToString());

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!AbilityMontage)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(AbilityMontage))
		{
			if (FirstSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(FirstSectionName, AbilityMontage);
			}

			for (int32 SectionIndex = 0; SectionIndex < AbilityMontage->CompositeSections.Num() - 1; ++SectionIndex)
			{
				const FName CurrentSectionName = AbilityMontage->GetSectionName(SectionIndex);
				const FName NextSectionName = AbilityMontage->GetSectionName(SectionIndex + 1);
				if (CurrentSectionName != NAME_None && NextSectionName != NAME_None)
				{
					AnimInstance->Montage_SetNextSection(CurrentSectionName, NextSectionName, AbilityMontage);
				}
			}
		}
	}
}

void UKDGA_ComboAttack::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("KDGA_ComboAttack: Full combo montage completed"));
	ResetComboCounter();
	CurrentSectionIndex = 0;
	bNextSectionQueued = false;
	Super::OnMontageCompleted();
}

void UKDGA_ComboAttack::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Log, TEXT("KDGA_ComboAttack: Combo montage interrupted"));
	ResetComboCounter();
	CurrentSectionIndex = 0;
	bNextSectionQueued = false;
	Super::OnMontageInterrupted();
}

void UKDGA_ComboAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetComboCounter();
	CurrentSectionIndex = 0;
	bNextSectionQueued = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
