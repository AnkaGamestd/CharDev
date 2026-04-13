// Copyright Kingdawn. All Rights Reserved.

#include "Abilities/KDGA_StatusEffect.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/KDStatsComponent.h"
#include "Components/KDTargetingComponent.h"
#include "Components/KDDamageComponent.h"
#include "Core/KDGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UKDGA_StatusEffect::UKDGA_StatusEffect()
{
	// FrostNova-style defaults
	AoERadius = 500.f;
	bSelfCentered = true;
	StatusEffect = nullptr;
	EffectDuration = 3.f;
	EffectMagnitude = 0.5f; // 50% slow by default
	StatusEffectTag = FGameplayTag();

	// Override ranged spell defaults for AoE CC
	ManaCost = 25.f;
	BaseDamage = 10.f; // FrostNova does minimal damage
	bRequiresTarget = false; // AoE doesn't need a single target
	MaxRange = 500.f; // Self-centered range
	bAutoFaceTarget = false;
}

void UKDGA_StatusEffect::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Let the parent chain handle: definition overrides, commit, montage playback,
	// and gameplay event waiting (if montage has a Fire notify).
	// UKDGA_RangedSpell::ActivateAbility() -> UKDGameplayAbility::ActivateAbility()
	// will play the montage and set up event listeners.
	// FireSpell_Implementation() (overridden below) is called when the spell fires.
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// If there's no montage, Super already called FireSpell() and EndAbility().
	// If there IS a montage, Super set up the event listener and the spell
	// fires when the montage's KD.Ability.Event.Fire notify triggers.
}

void UKDGA_StatusEffect::FireSpell_Implementation()
{
	if (!CurrentActorInfo || !CurrentActorInfo->OwnerActor.IsValid())
	{
		return;
	}

	AActor* Owner = CurrentActorInfo->OwnerActor.Get();

	// Determine AoE center
	FVector Center;
	if (bSelfCentered)
	{
		Center = Owner->GetActorLocation();
	}
	else
	{
		AActor* Target = GetAbilityTarget();
		Center = Target ? Target->GetActorLocation() : Owner->GetActorLocation();
	}

	// --- Single overlap pass with deduplication ---
	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(AoERadius);
	World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Pawn, SphereShape, QueryParams);

	// Deduplicate by actor pointer — OverlapMultiByChannel returns one result per
	// overlapping component, so a single character with capsule + mesh + weapon
	// can produce 3+ results for the same actor.
	TSet<AActor*> ProcessedActors;

	int32 AffectedCount = 0;
	int32 SkippedNoASC = 0;
	int32 SkippedImmune = 0;

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* TargetActor = Result.GetActor();
		if (!TargetActor || TargetActor == Owner || ProcessedActors.Contains(TargetActor))
		{
			continue;
		}
		ProcessedActors.Add(TargetActor);

		// Filter: only process actors that have an AbilitySystemComponent.
		// This skips static meshes, environment props, and any non-combat actors.
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
		if (!TargetASC)
		{
			SkippedNoASC++;
			continue;
		}

		// Immunity check
		if (IsTargetImmune(TargetActor, TargetASC))
		{
			SkippedImmune++;
			continue;
		}

		// Apply status effect GE
		if (StatusEffect)
		{
			ApplyStatusEffectGEToTarget(TargetActor, TargetASC);
		}

		// Apply base spell damage through KDDamageComponent pipeline
		// This ensures AoE spells use the same damage pipeline as other attacks
		// (formula scaling + immortality guard + event delegates)
		if (BaseDamage > 0.f)
		{
			ApplySpellDamageToTarget(TargetActor, BaseDamage, Owner);
		}

		AffectedCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("KDGA_StatusEffect: %s at (%.0f,%.0f,%.0f) r=%.0f — %d affected, %d skipped (no ASC), %d immune"),
		*GetName(), Center.X, Center.Y, Center.Z, AoERadius,
		AffectedCount, SkippedNoASC, SkippedImmune);

	// Notify Blueprint for VFX
	BP_OnStatusEffectApplied(Center, AoERadius);
}

bool UKDGA_StatusEffect::IsTargetImmune(AActor* Target, UAbilitySystemComponent* TargetASC)
{
	if (!StatusEffectTag.IsValid())
	{
		return false;
	}

	// Map status effect tags to their immunity counterparts
	static const TMap<FName, FName> EffectToImmunity = {
		{ FName("KD.Status.Effect.Knockdown"), FName("KD.Immunity.Knockdown") },
		{ FName("KD.Status.Effect.Stun"),      FName("KD.Immunity.Stun") },
		{ FName("KD.Status.Effect.Slow"),       FName("KD.Immunity.Slow") },
		{ FName("KD.Status.Effect.Freeze"),     FName("KD.Immunity.Freeze") },
	};

	if (const FName* ImmunityTagName = EffectToImmunity.Find(StatusEffectTag.GetTagName()))
	{
		FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(*ImmunityTagName, false);
		if (ImmunityTag.IsValid() && TargetASC->HasMatchingGameplayTag(ImmunityTag))
		{
			return true;
		}
	}

	return false;
}

void UKDGA_StatusEffect::ApplyStatusEffectGEToTarget(AActor* Target, UAbilitySystemComponent* TargetASC)
{
	UAbilitySystemComponent* OwnerASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!OwnerASC || !Target || !TargetASC)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(
		StatusEffect, 1, OwnerASC->MakeEffectContext());

	if (!SpecHandle.IsValid())
	{
		return;
	}

	if (EffectDuration > 0.f)
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(
			FKDGameplayTags::Data_Duration, EffectDuration);
	}

	if (EffectMagnitude > 0.f)
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(
			FKDGameplayTags::Data_Magnitude, EffectMagnitude);
	}

	OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	FString DebugLabel = TEXT("STATUS");
	if (StatusEffectTag.MatchesTagExact(FKDGameplayTags::Status_Effect_Slow))
	{
		DebugLabel = TEXT("SLOW");
	}
	else if (StatusEffectTag.MatchesTagExact(FKDGameplayTags::Status_Effect_Freeze))
	{
		DebugLabel = TEXT("FREEZE");
	}
	else if (StatusEffectTag.MatchesTagExact(FKDGameplayTags::Status_Effect_Knockdown))
	{
		DebugLabel = TEXT("KD");
	}
	else if (StatusEffectTag.MatchesTagExact(FKDGameplayTags::Status_Effect_Bleed))
	{
		DebugLabel = TEXT("BLEED");
	}

	if (UWorld* World = Target->GetWorld())
	{
		DrawDebugString(World, Target->GetActorLocation() + FVector(0.f, 0.f, 140.f), DebugLabel, nullptr, FColor(80, 170, 255), 1.5f, false, 1.4f);
	}
}
