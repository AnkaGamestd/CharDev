// Copyright Kingdawn. All Rights Reserved.

#include "Components/KDAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Core/KDGameplayTags.h"
#include "Mastery/KDMasteryComponent.h"
#include "Mastery/KDSkillDefinition.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

namespace
{
	FGameplayTag ResolveBasicAttackTag()
	{
		if (FKDGameplayTags::Combat_Action_Attack_Basic.IsValid())
		{
			return FKDGameplayTags::Combat_Action_Attack_Basic;
		}

		return FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Combat.Action.Attack.Basic")), false);
	}

	float GetCooldownTimeSeconds(const UWorld* World)
	{
		return World ? World->GetTimeSeconds() : 0.f;
	}
}

UKDAbilitySystemComponent::UKDAbilitySystemComponent()
{
	bIsPerformingAbility = false;
	bMasteryEventsBound = false;
}

void UKDAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind to mastery component events for automatic reconciliation.
	// This ensures that when skills are unlocked or masteries are activated/deactivated,
	// the ASC automatically grants or revokes the corresponding abilities.
	if (!bMasteryEventsBound)
	{
		if (UKDMasteryComponent* MasteryComp = GetMasteryComponent())
		{
			MasteryComp->OnSkillUnlocked.AddDynamic(this, &UKDAbilitySystemComponent::OnMasterySkillUnlocked);
			MasteryComp->OnMasteryActivated.AddDynamic(this, &UKDAbilitySystemComponent::OnMasteryActivated);
			MasteryComp->OnMasteryDeactivated.AddDynamic(this, &UKDAbilitySystemComponent::OnMasteryDeactivated);
			bMasteryEventsBound = true;
			
			UE_LOG(LogTemp, Log, TEXT("KDASC: Bound to mastery component events on %s"), *GetNameSafe(GetOwner()));
		}
	}
}

bool UKDAbilitySystemComponent::TriggerAbilityByTag(FGameplayTag AbilityTag)
{
	if (!AbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("KDASC: TriggerAbilityByTag failed - invalid tag"));
		return false;
	}

	const float CooldownRemaining = GetAbilityTagCooldownRemaining(AbilityTag);
	if (CooldownRemaining > 0.f)
	{
		UE_LOG(LogTemp, Log, TEXT("KDASC: TriggerAbilityByTag blocked - %s still on cooldown for %.2fs"),
			*AbilityTag.ToString(),
			CooldownRemaining);
		return false;
	}

	if (bIsPerformingAbility)
	{
		const FGameplayTag BasicAttackTag = ResolveBasicAttackTag();
		const bool bCurrentIsBasic = BasicAttackTag.IsValid() && CurrentAbilityTag == BasicAttackTag;
		const bool bNewIsBasic = BasicAttackTag.IsValid() && AbilityTag == BasicAttackTag;

		// Allow cancellation in two cases:
		// 1. Basic attack → non-basic (hotbar skills override filler loop)
		// 2. Non-basic → non-basic (standard MMO spell queueing behavior)
		const bool bShouldCancelCurrent = (bCurrentIsBasic && !bNewIsBasic) || (!bCurrentIsBasic && !bNewIsBasic);

		if (bShouldCancelCurrent)
		{
			const FGameplayAbilitySpecHandle CurrentHandle = GetAbilityHandleByTag(CurrentAbilityTag);
			if (CurrentHandle.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("KDASC: Canceling active ability %s to allow %s"),
					*CurrentAbilityTag.ToString(),
					*AbilityTag.ToString());
				CancelAbilityHandle(CurrentHandle);
			}

			// Allow the new ability to proceed (concurrency gate opens after cancel starts)
			bIsPerformingAbility = false;
			CurrentAbilityTag = FGameplayTag();
		}
		else
		{
			return false;
		}
	}

	FGameplayAbilitySpecHandle Handle = GetAbilityHandleByTag(AbilityTag);
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("KDASC: TriggerAbilityByTag - no spec for tag %s"), *AbilityTag.ToString());
		return false;
	}

	const bool bActivated = TryActivateAbility(Handle);
	if (bActivated)
	{
		bIsPerformingAbility = true;
		CurrentAbilityTag = AbilityTag;
		OnAbilityStartedEvent.Broadcast(CurrentAbilityTag);
	}
	return bActivated;
}

bool UKDAbilitySystemComponent::IsAbilityTagOnCooldown(FGameplayTag AbilityTag) const
{
	return GetAbilityTagCooldownRemaining(AbilityTag) > 0.f;
}

float UKDAbilitySystemComponent::GetAbilityTagCooldownRemaining(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		return 0.f;
	}

	CleanupExpiredCooldowns();

	const float* CooldownEndTime = AbilityCooldownEndTimes.Find(AbilityTag);
	if (!CooldownEndTime)
	{
		return 0.f;
	}

	return FMath::Max(0.f, *CooldownEndTime - GetCooldownTimeSeconds(GetWorld()));
}

void UKDAbilitySystemComponent::StartAbilityTagCooldown(FGameplayTag AbilityTag, float DurationSeconds)
{
	if (!AbilityTag.IsValid() || DurationSeconds <= 0.f)
	{
		return;
	}

	AbilityCooldownEndTimes.Add(AbilityTag, GetCooldownTimeSeconds(GetWorld()) + DurationSeconds);
}

void UKDAbilitySystemComponent::ClearAbilityTagCooldown(FGameplayTag AbilityTag)
{
	if (!AbilityTag.IsValid())
	{
		return;
	}

	AbilityCooldownEndTimes.Remove(AbilityTag);
}

void UKDAbilitySystemComponent::SendGameplayEvent(const FGameplayTag& GameplayEvent)
{
	FGameplayEventData Payload;
	HandleGameplayEvent(GameplayEvent, &Payload);
}

FGameplayAbilitySpecHandle UKDAbilitySystemComponent::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass,
	FGameplayTag AbilityTag, int32 InputID)
{
	if (!AbilityClass)
	{
		return FGameplayAbilitySpecHandle();
	}

	FGameplayAbilitySpec Spec(AbilityClass, 1, InputID, this);
	
	// Store the tag in the spec's DynamicAbilityTags
	// Note: DynamicSpecSourceTags is read-only via GetDynamicSpecSourceTags()
	// Suppress deprecation warning - this is the correct write path for UE 5.7
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	Spec.DynamicAbilityTags.AddTag(AbilityTag);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	const FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
	UE_LOG(LogTemp, Log, TEXT("KDASC: Granted ability %s with tag %s -> HandleValid=%d"),
		*GetNameSafe(AbilityClass),
		*AbilityTag.ToString(),
		Handle.IsValid());
	return Handle;
}

void UKDAbilitySystemComponent::RemoveAbilityByTag(FGameplayTag AbilityTag)
{
	FGameplayAbilitySpecHandle Handle = GetAbilityHandleByTag(AbilityTag);
	if (Handle.IsValid())
	{
		ClearAbility(Handle);
	}

	ClearAbilityTagCooldown(AbilityTag);
}

UGameplayAbility* UKDAbilitySystemComponent::GetAbilityByTag(const FGameplayTag& AbilityTag) const
{
	FGameplayAbilitySpecHandle Handle = GetAbilityHandleByTag(AbilityTag);
	if (Handle.IsValid())
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		if (Spec)
		{
			if (UGameplayAbility* PrimaryInstance = Spec->GetPrimaryInstance())
			{
				return PrimaryInstance;
			}

			return Spec->Ability;
		}
	}
	return nullptr;
}

FGameplayAbilitySpecHandle UKDAbilitySystemComponent::GetAbilityHandleByTag(const FGameplayTag& AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		return FGameplayAbilitySpecHandle();
	}

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTag(AbilityTag))
		{
			return Spec.Handle;
		}
	}

	return FGameplayAbilitySpecHandle();
}

void UKDAbilitySystemComponent::SetComboCounter(const FGameplayTag& ComboTag, int32 Value)
{
	ComboCounters.Add(ComboTag, Value);
}

int32 UKDAbilitySystemComponent::GetComboCount(const FGameplayTag& ComboTag) const
{
	const int32* Count = ComboCounters.Find(ComboTag);
	return Count ? *Count : 0;
}

void UKDAbilitySystemComponent::ResetComboCount(const FGameplayTag& ComboTag)
{
	ComboCounters.Remove(ComboTag);
}

void UKDAbilitySystemComponent::NotifyAbilityStarted(UGameplayAbility* Ability)
{
	if (!Ability)
	{
		return;
	}

	bIsPerformingAbility = true;

	// Extract tag from ability's DynamicSpecSourceTags (UE 5.7 API)
	const FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(Ability->GetClass());
	if (Spec && Spec->GetDynamicSpecSourceTags().Num() > 0)
	{
		CurrentAbilityTag = Spec->GetDynamicSpecSourceTags().First();
	}

	OnAbilityStartedEvent.Broadcast(CurrentAbilityTag);
}

void UKDAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	FGameplayTag EndedAbilityTag = CurrentAbilityTag;
	if (FGameplayAbilitySpec* EndedSpec = FindAbilitySpecFromHandle(Handle))
	{
		if (EndedSpec->GetDynamicSpecSourceTags().Num() > 0)
		{
			EndedAbilityTag = EndedSpec->GetDynamicSpecSourceTags().First();
		}
	}

	bIsPerformingAbility = false;

	OnAbilityFinishedEvent.Broadcast(EndedAbilityTag);

	// If the finished-ability callback started a new ability immediately (for example
	// Silkroad-style basic attack auto-repeat), do not wipe the newly-established state.
	if (!bIsPerformingAbility)
	{
		CurrentAbilityTag = FGameplayTag();
	}
}

// ========================================================================
// Mastery Integration (read-only queries)
// ========================================================================

UKDMasteryComponent* UKDAbilitySystemComponent::GetMasteryComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}
	return Owner->FindComponentByClass<UKDMasteryComponent>();
}

bool UKDAbilitySystemComponent::IsMasterySkillUnlocked(FGameplayTag SkillTag) const
{
	const UKDMasteryComponent* MasteryComp = GetMasteryComponent();
	if (!MasteryComp)
	{
		return false;
	}
	return MasteryComp->IsSkillUnlocked(SkillTag);
}

TArray<FGameplayTag> UKDAbilitySystemComponent::GetAllMasteryUnlockedSkills() const
{
	const UKDMasteryComponent* MasteryComp = GetMasteryComponent();
	if (!MasteryComp)
	{
		return TArray<FGameplayTag>();
	}
	return MasteryComp->GetAllUnlockedSkills();
}

// ========================================================================
// Mastery Grant Reconciliation (with grant-source tracking + safe revoke)
// ========================================================================

FKDMasteryReconciliationResult UKDAbilitySystemComponent::ReconcileMasteryGrantedAbilities()
{
	FKDMasteryReconciliationResult Result;

	UKDMasteryComponent* MasteryComp = GetMasteryComponent();
	if (!MasteryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDASC::ReconcileMastery: No mastery component found on %s. Skipping."),
			*GetNameSafe(GetOwner()));
		return Result;
	}

	// 1. Collect all mastery-unlocked skill tags
	const TArray<FGameplayTag> UnlockedSkills = MasteryComp->GetAllUnlockedSkills();
	const TSet<FGameplayTag> UnlockedSet(UnlockedSkills);

	UE_LOG(LogTemp, Log, TEXT("KDASC::ReconcileMastery: Starting reconciliation. %d mastery-unlocked skills, %d mastery-tracked grants."),
		UnlockedSkills.Num(), MasteryGrantedSkills.Num());

	// 2. Build a set of currently granted skill tags from the ASC's activatable abilities.
	TSet<FGameplayTag> GrantedSkillTags;
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		const FGameplayTagContainer& Tags = Spec.GetDynamicSpecSourceTags();
		for (const FGameplayTag& Tag : Tags)
		{
			if (Tag.IsValid())
			{
				GrantedSkillTags.Add(Tag);
			}
		}
	}

	// 3. Grant missing abilities: skills unlocked in mastery but not yet granted.
	for (const FGameplayTag& SkillTag : UnlockedSkills)
	{
		if (GrantedSkillTags.Contains(SkillTag))
		{
			// Already granted by some path (build, test, or previous mastery reconciliation).
			// We do NOT claim ownership of pre-existing grants. If it was previously
			// mastery-granted it will already be in MasteryGrantedSkills. If it was
			// build-granted, we must not add it to our tracking set.
			continue;
		}

		// Resolve skill tag -> ability class via KDSkillDefinition (new mastery path).
		UKDSkillDefinition* SkillDef = MasteryComp->FindSkillDefinition(SkillTag);
		if (!SkillDef)
		{
			UE_LOG(LogTemp, Warning, TEXT("KDASC::ReconcileMastery: No skill definition found for unlocked skill %s. Skipping grant."),
				*SkillTag.ToString());
			Result.FailedSkills.Add(SkillTag);
			continue;
		}

		if (SkillDef->AbilityClass.IsNull())
		{
			UE_LOG(LogTemp, Warning, TEXT("KDASC::ReconcileMastery: Skill %s has no ability class configured. Skipping grant."),
				*SkillTag.ToString());
			Result.FailedSkills.Add(SkillTag);
			continue;
		}

		// Load the ability class (synchronous - acceptable for a discrete event).
		TSubclassOf<UGameplayAbility> AbilityClassLoaded = SkillDef->AbilityClass.LoadSynchronous();
		if (!AbilityClassLoaded)
		{
			UE_LOG(LogTemp, Warning, TEXT("KDASC::ReconcileMastery: Failed to load ability class for skill %s. Skipping grant."),
				*SkillTag.ToString());
			Result.FailedSkills.Add(SkillTag);
			continue;
		}

		// Grant the ability and record mastery ownership.
		const FGameplayAbilitySpecHandle Handle = GrantAbility(AbilityClassLoaded, SkillTag);
		if (Handle.IsValid())
		{
			MasteryGrantedSkills.Add(SkillTag);
			Result.GrantedSkills.Add(SkillTag);
			UE_LOG(LogTemp, Log, TEXT("KDASC::ReconcileMastery: Granted + tracked mastery skill %s (%s)."),
				*SkillTag.ToString(),
				*GetNameSafe(AbilityClassLoaded));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("KDASC::ReconcileMastery: GrantAbility returned invalid handle for skill %s."),
				*SkillTag.ToString());
			Result.FailedSkills.Add(SkillTag);
		}
	}

	// 4. Revoke stale mastery-granted abilities.
	//    ONLY abilities in MasteryGrantedSkills can be revoked here.
	//    This guarantees we never revoke build-granted or test-granted abilities.
	TArray<FGameplayTag> TagsToRevoke;
	for (const FGameplayTag& TrackedTag : MasteryGrantedSkills)
	{
		if (!UnlockedSet.Contains(TrackedTag))
		{
			TagsToRevoke.Add(TrackedTag);
		}
	}

	for (const FGameplayTag& StaleTag : TagsToRevoke)
	{
		// Verify the ability is actually still granted (could have been removed externally).
		const FGameplayAbilitySpecHandle Handle = GetAbilityHandleByTag(StaleTag);
		if (Handle.IsValid())
		{
			RemoveAbilityByTag(StaleTag);
			UE_LOG(LogTemp, Log, TEXT("KDASC::ReconcileMastery: Revoked mastery-granted ability %s (no longer mastery-unlocked)."),
				*StaleTag.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("KDASC::ReconcileMastery: Mastery-tracked skill %s was already removed externally. Cleaning up tracking."),
				*StaleTag.ToString());
		}

		MasteryGrantedSkills.Remove(StaleTag);
		Result.RevokedSkills.Add(StaleTag);
	}

	UE_LOG(LogTemp, Log, TEXT("KDASC::ReconcileMastery: Reconciliation complete. Granted=%d, Revoked=%d, Failed=%d, Tracked=%d."),
		Result.GrantedSkills.Num(),
		Result.RevokedSkills.Num(),
		Result.FailedSkills.Num(),
		MasteryGrantedSkills.Num());

	return Result;
}

bool UKDAbilitySystemComponent::IsMasteryGrantedSkill(FGameplayTag SkillTag) const
{
	return MasteryGrantedSkills.Contains(SkillTag);
}

const TSet<FGameplayTag>& UKDAbilitySystemComponent::GetMasteryGrantedSkills() const
{
	return MasteryGrantedSkills;
}

void UKDAbilitySystemComponent::CleanupExpiredCooldowns() const
{
	UKDAbilitySystemComponent* MutableThis = const_cast<UKDAbilitySystemComponent*>(this);
	const float CurrentTime = GetCooldownTimeSeconds(GetWorld());
	for (auto It = MutableThis->AbilityCooldownEndTimes.CreateIterator(); It; ++It)
	{
		if (It.Value() <= CurrentTime)
		{
			It.RemoveCurrent();
		}
	}
}

// ========================================================================
// Mastery Event Handlers
// ========================================================================

void UKDAbilitySystemComponent::OnMasterySkillUnlocked(FGameplayTag SkillTag, FKDMasteryIdentity MasteryIdentity)
{
	UE_LOG(LogTemp, Log, TEXT("KDASC: OnMasterySkillUnlocked(%s) — triggering reconciliation"), *SkillTag.ToString());
	ReconcileMasteryGrantedAbilities();
}

void UKDAbilitySystemComponent::OnMasteryActivated(FKDMasteryIdentity MasteryIdentity)
{
	const FString IdentityText = MasteryIdentity.IsValid()
		? FString::Printf(TEXT("Class=%d Branch=%d"),
			static_cast<int32>(MasteryIdentity.MasteryClass),
			static_cast<int32>(MasteryIdentity.MasteryBranch))
		: TEXT("Invalid");
	UE_LOG(LogTemp, Log, TEXT("KDASC: OnMasteryActivated(%s) - triggering reconciliation"), *IdentityText);
	ReconcileMasteryGrantedAbilities();
}

void UKDAbilitySystemComponent::OnMasteryDeactivated(FKDMasteryIdentity MasteryIdentity)
{
	const FString IdentityText = MasteryIdentity.IsValid()
		? FString::Printf(TEXT("Class=%d Branch=%d"),
			static_cast<int32>(MasteryIdentity.MasteryClass),
			static_cast<int32>(MasteryIdentity.MasteryBranch))
		: TEXT("Invalid");
	UE_LOG(LogTemp, Log, TEXT("KDASC: OnMasteryDeactivated(%s) - triggering reconciliation"), *IdentityText);
	ReconcileMasteryGrantedAbilities();
}

void UKDAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UKDAbilitySystemComponent, bIsPerformingAbility);
	DOREPLIFETIME(UKDAbilitySystemComponent, CurrentAbilityTag);
}
