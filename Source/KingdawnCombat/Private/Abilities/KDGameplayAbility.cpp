// Copyright Kingdawn. All Rights Reserved.

#include "Abilities/KDGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Data/KDAbilityDefinition.h"
#include "Components/KDAbilitySystemComponent.h"
#include "Components/KDStatsComponent.h"
#include "Components/KDTargetingComponent.h"
#include "Interfaces/KDEntityInterface.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	float GetOwnerRangeMultiplier(const AActor* OwnerActor)
	{
		if (!OwnerActor || !OwnerActor->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass()))
		{
			return 1.f;
		}

		const FKDMasteryIdentity OwnerIdentity = IKDEntityInterface::Execute_GetEntityMasteryIdentity(const_cast<AActor*>(OwnerActor));
		return OwnerIdentity.MasteryBranch == EKDMasteryBranch::Wizard ? 1.5f : 1.f;
	}

	UKDAbilityDefinition* LoadAbilityDefinitionByPath(const TCHAR* Path)
	{
		return Cast<UKDAbilityDefinition>(FSoftObjectPath(Path).TryLoad());
	}
}

UKDGameplayAbility::UKDGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityDefinition = nullptr;
	StaminaCost = 0.f;
	ManaCost = 0.f;
	CooldownDuration = 0.f;
	bCommitOnActivate = true;
	bEndOnMontageComplete = true;
	MontagePlayRate = 1.f;
	bHasCommitted = false;
	bEndingAbility = false;

	// MMO targeting defaults
	bRequiresTarget = false;
	MinRange = 0.f;
	MaxRange = 300.f;
	bAutoFaceTarget = false;
	FacingInterpSpeed = 0.f; // 0 = instant
}

bool UKDGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const_cast<UKDGameplayAbility*>(this)->ApplyDefinitionOverrides();

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		UE_LOG(LogTemp, Log, TEXT("KDGA: Super::CanActivateAbility failed for %s"), *GetName());
		return false;
	}

	if (const UKDAbilitySystemComponent* KDASC = ActorInfo ? Cast<UKDAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()) : nullptr)
	{
		const float EffectiveCooldownDuration = GetCooldownDuration();
		const float CooldownRemaining = AbilityTag.IsValid() ? KDASC->GetAbilityTagCooldownRemaining(AbilityTag) : 0.f;
		if (EffectiveCooldownDuration > 0.f && CooldownRemaining > 0.f)
		{
			UE_LOG(LogTemp, Log, TEXT("KDGA: CanActivate failed for %s - cooldown %.2fs remaining"),
				*GetName(), CooldownRemaining);
			return false;
		}
	}

	// Check branch/weapon compatibility if a definition is assigned.
	// This is now a real runtime gate, not just documentation.
	if (AbilityDefinition)
	{
		if (AActor* OwnerActor = ActorInfo && ActorInfo->OwnerActor.IsValid() ? ActorInfo->OwnerActor.Get() : nullptr)
		{
			if (OwnerActor->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass()))
			{
				const FKDMasteryIdentity OwnerIdentity = IKDEntityInterface::Execute_GetEntityMasteryIdentity(OwnerActor);
				const EKDWeaponType OwnerWeapon = IKDEntityInterface::Execute_GetEntityWeaponType(OwnerActor);

				if (!AbilityDefinition->IsCompatibleWithMastery(OwnerIdentity, OwnerWeapon))
				{
					const int32 RequiredBranch = AbilityDefinition->Compatibility.AllowedMasteryBranches.Num() > 0
						? static_cast<int32>(AbilityDefinition->Compatibility.AllowedMasteryBranches[0])
						: -1;
					UE_LOG(LogTemp, Warning, TEXT("KDGA: CanActivate BLOCKED - %s incompatible (has MasteryClass=%d MasteryBranch=%d Weapon=%d, needs MasteryClass=%d MasteryBranch=%d)"),
						*GetName(),
						static_cast<int32>(OwnerIdentity.MasteryClass),
						static_cast<int32>(OwnerIdentity.MasteryBranch),
						static_cast<int32>(OwnerWeapon),
						static_cast<int32>(AbilityDefinition->Compatibility.RequiredMasteryClass),
						RequiredBranch);
					return false;
				}
			}
		}
	}

	// Check stamina cost
	if (StaminaCost > 0.f)
	{
		UKDStatsComponent* StatsComp = Cast<UKDStatsComponent>(ActorInfo->OwnerActor->FindComponentByClass<UKDStatsComponent>());
		if (!StatsComp || !StatsComp->CanAffordStaminaCost(StaminaCost))
		{
			UE_LOG(LogTemp, Log, TEXT("KDGA: CanActivate failed for %s - insufficient stamina (cost %.1f)"), *GetName(), StaminaCost);
			return false;
		}
	}

	// Check mana cost
	if (ManaCost > 0.f)
	{
		UKDStatsComponent* StatsComp = Cast<UKDStatsComponent>(ActorInfo->OwnerActor->FindComponentByClass<UKDStatsComponent>());
		if (!StatsComp || !StatsComp->CanAffordManaCost(ManaCost))
		{
			UE_LOG(LogTemp, Log, TEXT("KDGA: CanActivate failed for %s - insufficient mana (cost %.1f)"), *GetName(), ManaCost);
			return false;
		}
	}

	// Check target requirement (MMO-style)
	if (bRequiresTarget)
	{
		UKDTargetingComponent* TargetingComp = Cast<UKDTargetingComponent>(ActorInfo->OwnerActor->FindComponentByClass<UKDTargetingComponent>());
		if (!TargetingComp)
		{
			UE_LOG(LogTemp, Log, TEXT("KDGA: CanActivate failed for %s - no targeting component"), *GetName());
			return false;
		}

		AActor* Target = TargetingComp->GetValidTarget();
		if (!Target)
		{
			UE_LOG(LogTemp, Log, TEXT("KDGA: CanActivate failed for %s - no valid target"), *GetName());
			return false; // No valid target
		}

		// Check range
		const float RangeMultiplier = GetOwnerRangeMultiplier(ActorInfo->OwnerActor.Get());
		const float EffectiveMinRange = MinRange * RangeMultiplier;
		const float EffectiveMaxRange = MaxRange * RangeMultiplier;
		const float Distance = FVector::Dist(ActorInfo->OwnerActor->GetActorLocation(), Target->GetActorLocation());
		if (Distance < EffectiveMinRange || Distance > EffectiveMaxRange)
		{
			UE_LOG(LogTemp, Log, TEXT("KDGA: CanActivate failed for %s - target out of range (distance %.1f, min %.1f, max %.1f)"),
				*GetName(), Distance, EffectiveMinRange, EffectiveMaxRange);
			return false; // Out of range
		}
	}
	return true;
}

void UKDGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("KDGameplayAbility::ActivateAbility ENTERED for %s (class=%s)"),
		*GetName(), *GetClass()->GetName());

	ApplyDefinitionOverrides();

	UE_LOG(LogTemp, Log, TEXT("KDGameplayAbility::ActivateAbility - about to CommitAbility for %s"), *GetName());

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Error, TEXT("KDGA: CommitAbility failed for %s"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("KDGameplayAbility::ActivateAbility - CommitAbility succeeded for %s"), *GetName());

	bHasCommitted = true;

	// Consume stamina (from definition or manual config)
	const float EffectiveStaminaCost = AbilityDefinition ? AbilityDefinition->StaminaCost : StaminaCost;
	if (EffectiveStaminaCost > 0.f)
	{
		UKDStatsComponent* StatsComp = Cast<UKDStatsComponent>(ActorInfo->OwnerActor->FindComponentByClass<UKDStatsComponent>());
		if (StatsComp)
		{
			StatsComp->ConsumeStamina(EffectiveStaminaCost);
		}
	}

	// Consume mana (from definition or manual config)
	const float EffectiveManaCost = AbilityDefinition ? AbilityDefinition->ManaCost : ManaCost;
	if (EffectiveManaCost > 0.f)
	{
		UKDStatsComponent* StatsComp = Cast<UKDStatsComponent>(ActorInfo->OwnerActor->FindComponentByClass<UKDStatsComponent>());
		if (StatsComp)
		{
			StatsComp->ConsumeMana(EffectiveManaCost);
		}
	}

	if (bCommitOnActivate)
	{
		if (UKDAbilitySystemComponent* KDASC = GetKDAbilitySystemComponent())
		{
			const float EffectiveCooldownDuration = GetCooldownDuration();
			if (EffectiveCooldownDuration > 0.f && AbilityTag.IsValid())
			{
				KDASC->StartAbilityTagCooldown(AbilityTag, EffectiveCooldownDuration);
			}
		}
	}

	// Auto-face target before playing montage (MMO-style)
	if (bAutoFaceTarget)
	{
		FaceTarget();
	}

	UE_LOG(LogTemp, Log, TEXT("KDGA: %s post-override state: AbilityMontage=%s, AbilityDefinition=%s, AbilityTag=%s"),
		*GetName(),
		*GetNameSafe(AbilityMontage),
		*GetNameSafe(AbilityDefinition),
		AbilityTag.IsValid() ? *AbilityTag.ToString() : TEXT("None"));

	// Play montage if configured. If play fails, end immediately so the ability does not stay active.
	if (AbilityMontage)
	{
		if (!PlayAbilityMontage())
		{
			UE_LOG(LogTemp, Error, TEXT("KDGA: Montage play failed for %s - ending ability"), *GetName());
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDGA: No montage assigned for %s — subclass must handle FireSpell/EndAbility"), *GetName());
	}
}

void UKDGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bEndingAbility)
	{
		return;
	}

	bEndingAbility = true;

	// Stop montage if playing
	StopAbilityMontage();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	bHasCommitted = false;
	bEndingAbility = false;
}

// --- Montage Helpers ---

bool UKDGameplayAbility::PlayAbilityMontage()
{
	if (!AbilityMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("KDGA: PlayAbilityMontage failed for %s - no montage"), *GetName());
		return false;
	}

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("KDGA: PlayAbilityMontage failed for %s - no anim instance"), *GetName());

		// --- DIAGNOSTIC: Trace WHY AnimInstance is null ---
		if (CurrentActorInfo && CurrentActorInfo->OwnerActor.IsValid())
		{
			ACharacter* CharOwner = Cast<ACharacter>(CurrentActorInfo->OwnerActor.Get());
			if (CharOwner)
			{
				USkeletalMeshComponent* MeshComp = CharOwner->GetMesh();
				UE_LOG(LogTemp, Error, TEXT("  DIAG: Character=%s, Mesh=%s, MeshValid=%d, HasSkelMesh=%d, AnimMode=%d, AnimClass=%s"),
					*CharOwner->GetName(),
					*GetNameSafe(MeshComp),
					MeshComp != nullptr,
					MeshComp ? (MeshComp->GetSkeletalMeshAsset() != nullptr) : false,
					MeshComp ? static_cast<int32>(MeshComp->GetAnimationMode()) : -1,
					MeshComp ? *GetNameSafe(MeshComp->GetAnimClass()) : TEXT("null"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("  DIAG: OwnerActor=%s is NOT a Character"), *GetNameSafe(CurrentActorInfo->OwnerActor.Get()));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("  DIAG: CurrentActorInfo or OwnerActor is invalid/null"));
		}

		return false;
	}

	// --- DIAGNOSTIC: Log AnimInstance state before playing montage ---
	{
		ACharacter* CharOwner = Cast<ACharacter>(CurrentActorInfo->OwnerActor.Get());
		USkeletalMeshComponent* MeshComp = CharOwner ? CharOwner->GetMesh() : nullptr;
		const USkeleton* MontageSkeleton = AbilityMontage ? AbilityMontage->GetSkeleton() : nullptr;
		const USkeleton* MeshSkeleton = (MeshComp && MeshComp->GetSkeletalMeshAsset()) ? MeshComp->GetSkeletalMeshAsset()->GetSkeleton() : nullptr;
		const bool bSkeletonMatch = MontageSkeleton && MeshSkeleton && MontageSkeleton == MeshSkeleton;

		UE_LOG(LogTemp, Log, TEXT("KDGA: PlayAbilityMontage for %s - montage=%s, AnimInstance=%s, AnimMode=%d, AnimClass=%s, MontageSkeleton=%s, MeshSkeleton=%s, SkeletonMatch=%d"),
			*GetName(),
			*GetNameSafe(AbilityMontage),
			*GetNameSafe(AnimInstance),
			MeshComp ? static_cast<int32>(MeshComp->GetAnimationMode()) : -1,
			MeshComp ? *GetNameSafe(MeshComp->GetAnimClass()) : TEXT("null"),
			*GetNameSafe(MontageSkeleton),
			*GetNameSafe(MeshSkeleton),
			bSkeletonMatch);
	}

	// Create and bind montage task
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage, MontagePlayRate, NAME_None, false);

	MontageTask->OnCompleted.AddDynamic(this, &UKDGameplayAbility::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UKDGameplayAbility::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UKDGameplayAbility::OnMontageCancelled);
	MontageTask->OnBlendOut.AddDynamic(this, &UKDGameplayAbility::OnMontageBlendOut);

	MontageTask->ReadyForActivation();

	UE_LOG(LogTemp, Log, TEXT("KDGA: PlayAbilityMontage task created and activated for %s"), *GetName());

	return true;
}

void UKDGameplayAbility::StopAbilityMontage()
{
	if (MontageTask)
	{
		UAbilityTask_PlayMontageAndWait* TaskToEnd = MontageTask;
		MontageTask = nullptr;
		TaskToEnd->EndTask();
	}
}

UAnimInstance* UKDGameplayAbility::GetOwnerAnimInstance() const
{
	if (!CurrentActorInfo || !CurrentActorInfo->OwnerActor.IsValid())
	{
		return nullptr;
	}

	ACharacter* CharOwner = Cast<ACharacter>(CurrentActorInfo->OwnerActor.Get());
	if (CharOwner && CharOwner->GetMesh())
	{
		return CharOwner->GetMesh()->GetAnimInstance();
	}

	return nullptr;
}

UKDAbilitySystemComponent* UKDGameplayAbility::GetKDAbilitySystemComponent() const
{
	if (!CurrentActorInfo || !CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		return nullptr;
	}

	return Cast<UKDAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
}

float UKDGameplayAbility::GetCooldownDuration() const
{
	return AbilityDefinition ? AbilityDefinition->CooldownDuration : CooldownDuration;
}

void UKDGameplayAbility::ApplyDefinitionOverrides()
{
	UKDAbilityDefinition* ResolvedDefinition = AbilityDefinition;

	if (CurrentActorInfo && CurrentActorInfo->OwnerActor.IsValid())
	{
		if (AActor* OwnerActor = CurrentActorInfo->OwnerActor.Get())
		{
			const FString AbilityPath = GetClass()->GetPathName();
			const bool bIsSharedBasicAttackShell = AbilityPath.Contains(TEXT("/BP_GA_Attack.")) || AbilityPath.Contains(TEXT("/BP_GA_Attack_C")) || GetClass()->GetName() == TEXT("BP_GA_Attack_C");
			const bool bIsSharedComboShell = AbilityPath.Contains(TEXT("/BP_GA_ComboAttack.")) || AbilityPath.Contains(TEXT("/BP_GA_ComboAttack_C")) || GetClass()->GetName() == TEXT("BP_GA_ComboAttack_C");

			if (OwnerActor->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass()))
			{
				const FKDMasteryIdentity OwnerIdentity = IKDEntityInterface::Execute_GetEntityMasteryIdentity(OwnerActor);
				if (bIsSharedBasicAttackShell)
				{
					if (OwnerIdentity.MasteryBranch == EKDMasteryBranch::BladeShield)
					{
						ResolvedDefinition = LoadAbilityDefinitionByPath(TEXT("/Game/KingdawnCombat/Abilities/Combat/Skills/BladeShield/DA_BladeShield_BasicAttack.DA_BladeShield_BasicAttack"));
					}
					else if (OwnerIdentity.MasteryBranch == EKDMasteryBranch::Wizard)
					{
						ResolvedDefinition = LoadAbilityDefinitionByPath(TEXT("/Game/KingdawnCombat/Abilities/Combat/Skills/Wizard/DA_Wizard_BasicAttack.DA_Wizard_BasicAttack"));
					}
				}
				else if (bIsSharedComboShell && OwnerIdentity.MasteryBranch == EKDMasteryBranch::BladeShield)
				{
					ResolvedDefinition = LoadAbilityDefinitionByPath(TEXT("/Game/KingdawnCombat/Abilities/Combat/Skills/BladeShield/DA_BladeShield_ComboLight.DA_BladeShield_ComboLight"));
				}
			}
		}
	}

	if (!ResolvedDefinition)
	{
		return;
	}

	AbilityDefinition = ResolvedDefinition;
	AbilityTag = ResolvedDefinition->AbilityTag;
	AbilityMontage = ResolvedDefinition->AbilityMontage;
	StaminaCost = ResolvedDefinition->StaminaCost;
	ManaCost = ResolvedDefinition->ManaCost;
	CooldownDuration = ResolvedDefinition->CooldownDuration;
	bCommitOnActivate = ResolvedDefinition->bCommitOnActivate;
	bEndOnMontageComplete = ResolvedDefinition->bEndOnMontageComplete;
	MontagePlayRate = ResolvedDefinition->MontagePlayRate;
	bRequiresTarget = ResolvedDefinition->bRequiresTarget;
	MinRange = ResolvedDefinition->MinRange;
	MaxRange = ResolvedDefinition->MaxRange;
	bAutoFaceTarget = ResolvedDefinition->bAutoFaceTarget;
	FacingInterpSpeed = ResolvedDefinition->FacingInterpSpeed;

	// Cooldowns are handled in KDAbilitySystemComponent as lightweight runtime tag gates.
}

// --- Targeting Helpers ---

AActor* UKDGameplayAbility::GetAbilityTarget() const
{
	if (!CurrentActorInfo || !CurrentActorInfo->OwnerActor.IsValid())
	{
		return nullptr;
	}

	UKDTargetingComponent* TargetingComp = Cast<UKDTargetingComponent>(CurrentActorInfo->OwnerActor->FindComponentByClass<UKDTargetingComponent>());
	if (TargetingComp)
	{
		return TargetingComp->GetValidTarget();
	}

	return nullptr;
}

bool UKDGameplayAbility::IsTargetInRange() const
{
	AActor* Target = GetAbilityTarget();
	if (!Target || !CurrentActorInfo || !CurrentActorInfo->OwnerActor.IsValid())
	{
		return false;
	}

	const float Distance = FVector::Dist(CurrentActorInfo->OwnerActor->GetActorLocation(), Target->GetActorLocation());
	return Distance >= MinRange && Distance <= MaxRange;
}

void UKDGameplayAbility::FaceTarget()
{
	AActor* Target = GetAbilityTarget();
	if (!Target || !CurrentActorInfo || !CurrentActorInfo->OwnerActor.IsValid())
	{
		return;
	}

	AActor* Owner = CurrentActorInfo->OwnerActor.Get();
	const FVector Direction = Target->GetActorLocation() - Owner->GetActorLocation();

	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation = FRotator(0.f, Direction.Rotation().Yaw, 0.f);

	if (FacingInterpSpeed > 0.f)
	{
		// Smooth rotation
		const FRotator NewRotation = FMath::RInterpTo(Owner->GetActorRotation(), TargetRotation, 0.016f, FacingInterpSpeed);
		Owner->SetActorRotation(NewRotation);
	}
	else
	{
		// Instant rotation
		Owner->SetActorRotation(TargetRotation);
	}
}

// --- Montage Callbacks ---

void UKDGameplayAbility::OnMontageCompleted()
{
	if (bEndingAbility)
	{
		return;
	}

	MontageTask = nullptr;
	BP_OnMontageCompleted();

	if (bEndOnMontageComplete)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
void UKDGameplayAbility::OnMontageInterrupted()
{
	if (bEndingAbility)
	{
		return;
	}

	MontageTask = nullptr;
	BP_OnMontageInterrupted();

	if (bEndOnMontageComplete)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UKDGameplayAbility::OnMontageCancelled()
{
	if (bEndingAbility)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("KDGA: Montage cancelled/failed for %s — ending ability"), *GetName());
	MontageTask = nullptr;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKDGameplayAbility::OnMontageBlendOut()
{
	// BlendOut is expected during normal montage completion; treat like Completed.
	OnMontageCompleted();
}
