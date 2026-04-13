// Copyright Kingdawn. All Rights Reserved.

#include "Components/KDDamageComponent.h"
#include "Components/KDStatsComponent.h"
#include "Interfaces/KDEntityInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"

namespace
{
	enum class EKDDamageScalingMode : uint8
	{
		Physical,
		Arcane,
		Hybrid
	};

	EKDDamageScalingMode ResolveDamageScalingMode(const AActor* DamageCauser)
	{
		if (!DamageCauser || !DamageCauser->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass()))
		{
			return EKDDamageScalingMode::Hybrid;
		}

		const FKDMasteryIdentity MasteryIdentity = IKDEntityInterface::Execute_GetEntityMasteryIdentity(const_cast<AActor*>(DamageCauser));
		switch (MasteryIdentity.MasteryBranch)
		{
		case EKDMasteryBranch::Wizard:
		case EKDMasteryBranch::Warlock:
		case EKDMasteryBranch::Necromancer:
		case EKDMasteryBranch::Cleric:
		case EKDMasteryBranch::Buffer:
			return EKDDamageScalingMode::Arcane;
		case EKDMasteryBranch::BladeShield:
		case EKDMasteryBranch::TwoHandedSword:
		case EKDMasteryBranch::Hammer:
		case EKDMasteryBranch::DualAxe:
		case EKDMasteryBranch::Glaive:
		case EKDMasteryBranch::SpearScythe:
		case EKDMasteryBranch::Dagger:
		case EKDMasteryBranch::Archer:
			return EKDDamageScalingMode::Physical;
		default:
			return EKDDamageScalingMode::Hybrid;
		}
	}

	void TryAddDamageTag(FGameplayTagContainer& TagContainer, const TCHAR* TagName)
	{
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TagName), false);
		if (Tag.IsValid())
		{
			TagContainer.AddTag(Tag);
		}
	}
}

UKDDamageComponent::UKDDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bIsAlive = true;
	bIsImmortal = false;

	// SILKROAD REMASTERED COMBAT RULE:
	// Normal damage does NOT interrupt or play hit reactions by default.
	// Only explicit status effects (stun, knockback, etc.) should interrupt.
	// bPlayHitReactions is now OPT-IN for specific cases, not universal.
	bPlayHitReactions = false;
	bEnableRagdollOnDeath = true;
}

void UKDDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind to stats component's death event
	UKDStatsComponent* StatsComp = GetOwner()->FindComponentByClass<UKDStatsComponent>();
	if (StatsComp && !StatsComp->OnHealthReachedZero.IsAlreadyBound(this, &UKDDamageComponent::HandleStatReachedZero))
	{
		StatsComp->OnHealthReachedZero.AddDynamic(this, &UKDDamageComponent::HandleStatReachedZero);
	}
}

float UKDDamageComponent::TakePointDamage(float Damage, FVector HitLocation, FVector HitNormal,
	FName BoneName, FVector ShotFromDirection, AController* InstigatedBy, AActor* DamageCauser, const FHitResult& HitInfo)
{
	FPointDamageEvent NewEvent(Damage, HitInfo, ShotFromDirection, UDamageType::StaticClass());
	return TakeDamage(GetOwner(), Damage, NewEvent, InstigatedBy, DamageCauser);
}

float UKDDamageComponent::ApplyAbilityDamage(float RawDamage, AActor* DamageCauser, AController* InstigatedBy)
{
	// Immortality and death guard
	if (!GetIsAlive() || GetIsImmortal())
	{
		return 0.f;
	}

	AActor* DamageReceiver = GetOwner();
	if (!DamageReceiver)
	{
		return 0.f;
	}

	// Calculate damage using centralized formula
	FGameplayTagContainer DamageTags;
	const float CalculatedDamage = CalculateDamageWithTags(RawDamage, DamageCauser, DamageReceiver, InstigatedBy, DamageTags);

	if (CalculatedDamage <= 0.f)
	{
		return 0.f;
	}

	// Construct damage event (using simplified hit info for ability damage)
	FHitResult SyntheticHitInfo;
	SyntheticHitInfo.HitObjectHandle = FActorInstanceHandle(DamageReceiver);
	SyntheticHitInfo.Location = DamageReceiver->GetActorLocation();
	SyntheticHitInfo.BoneName = NAME_None;

	FVector ShotDirection = FVector::ZeroVector;
	if (DamageCauser && DamageReceiver)
	{
		ShotDirection = (DamageReceiver->GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
	}

	FDamageEvent GenericDamageEvent;
	GenericDamageEvent.DamageTypeClass = UDamageType::StaticClass();

	// Build damage event struct
	FKDDamageEvent TempEvent;
	TempEvent.FinalDamage = CalculatedDamage;
	TempEvent.bIsCritical = false; // v1: no crit logic yet
	TempEvent.DamageReceiver = DamageReceiver;
	TempEvent.DamageDealer = DamageCauser;
	TempEvent.HitBoneName = NAME_None;
	TempEvent.HitLocation = SyntheticHitInfo.Location;
	TempEvent.HitResult = SyntheticHitInfo;
	TempEvent.DamageDirection = ShotDirection;
	TempEvent.DamageTags = DamageTags;

	LastDamageReceived = TempEvent;

	// Apply damage via stats component
	UKDStatsComponent* StatsComp = DamageReceiver->FindComponentByClass<UKDStatsComponent>();
	if (StatsComp)
	{
		StatsComp->ReceiveDamage(CalculatedDamage, InstigatedBy, DamageCauser);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDDamageComponent::ApplyAbilityDamage - %s has no KDStatsComponent!"), *DamageReceiver->GetName());
	}

	// Play hit reaction if enabled (Silkroad rule: false by default)
	if (bPlayHitReactions)
	{
		PlayHitReaction(LastDamageReceived);
	}

	// Broadcast events (OnDamageReceived for victim, OnDamageInflicted for dealer)
	OnRep_LastDamageReceived();

	return CalculatedDamage;
}

float UKDDamageComponent::TakeDamage(AActor* DamageReceiver, float Damage, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!GetIsAlive() || GetIsImmortal())
	{
		return 0.f;
	}

	if (!DamageReceiver)
	{
		return Damage;
	}

	// Extract hit info
	FHitResult OutDamage;
	FVector ShotDirection;
	DamageEvent.GetBestHitInfo(DamageReceiver, DamageCauser, OutDamage, ShotDirection);

	// Construct damage event
	ConstructDamageEvent(DamageReceiver, Damage, EventInstigator, OutDamage, ShotDirection, DamageEvent, DamageCauser);

	// Apply damage via stats component
	UKDStatsComponent* StatsComp = DamageReceiver->FindComponentByClass<UKDStatsComponent>();
	if (StatsComp)
	{
		StatsComp->ReceiveDamage(LastDamageReceived.FinalDamage, EventInstigator, DamageCauser);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("KDCombat: %s has no KDStatsComponent!"), *DamageReceiver->GetName());
	}

	// Play hit reaction montage
	if (bPlayHitReactions)
	{
		PlayHitReaction(LastDamageReceived);
	}

	// Broadcast events
	OnRep_LastDamageReceived();

	return LastDamageReceived.FinalDamage;
}

void UKDDamageComponent::Revive()
{
	bIsAlive = true;

	UKDStatsComponent* StatsComp = GetOwner()->FindComponentByClass<UKDStatsComponent>();
	if (StatsComp)
	{
		StatsComp->RestoreAllStats();
	}

	// TODO: Handle revive-specific logic (e.g., clear ragdoll, reset state)
}

void UKDDamageComponent::TriggerDeath()
{
	if (!bIsAlive)
	{
		return;
	}

	HandleStatReachedZero();
}

void UKDDamageComponent::ConstructDamageEvent(AActor* DamagedActor, float Damage, AController* InstigatedBy,
	const FHitResult& HitInfo, FVector ShotFromDirection, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
	FKDDamageEvent TempEvent;
	TempEvent.bIsCritical = false;
	TempEvent.FinalDamage = CalculateFinalDamage(Damage, DamagedActor, InstigatedBy, DamageCauser, DamageEvent, HitInfo, TempEvent.bIsCritical, TempEvent.DamageTags);
	TempEvent.DamageReceiver = DamagedActor;
	TempEvent.DamageDealer = DamageCauser;
	TempEvent.HitBoneName = HitInfo.BoneName;
	TempEvent.HitLocation = HitInfo.Location;
	TempEvent.HitResult = HitInfo;
	TempEvent.HitResult.HitObjectHandle = FActorInstanceHandle(DamagedActor);

	if (DamageCauser)
	{
		TempEvent.DamageDirection = ShotFromDirection.GetSafeNormal();
	}
	LastDamageReceived = TempEvent;
}

float UKDDamageComponent::CalculateDamage(float RawDamage, AActor* DamageCauser, AActor* DamagedActor,
	AController* InstigatedBy)
{
	// Wrapper for Blueprint-callable version without tags output
	FGameplayTagContainer UnusedTags;
	return CalculateDamageWithTags(RawDamage, DamageCauser, DamagedActor, InstigatedBy, UnusedTags);
}

float UKDDamageComponent::CalculateDamageWithTags(float RawDamage, AActor* DamageCauser, AActor* DamagedActor,
	AController* InstigatedBy, FGameplayTagContainer& OutDamageTags)
{
	if (RawDamage <= 0.f || !DamagedActor)
	{
		return 0.f;
	}

	const UKDStatsComponent* AttackerStats = nullptr;
	if (DamageCauser)
	{
		AttackerStats = DamageCauser->FindComponentByClass<UKDStatsComponent>();
	}
	else if (InstigatedBy && InstigatedBy->GetPawn())
	{
		AttackerStats = InstigatedBy->GetPawn()->FindComponentByClass<UKDStatsComponent>();
	}

	const UKDStatsComponent* DefenderStats = DamagedActor->FindComponentByClass<UKDStatsComponent>();
	const EKDDamageScalingMode ScalingMode = ResolveDamageScalingMode(DamageCauser);

	float OffenseMultiplier = 1.f;
	if (AttackerStats)
	{
		const float MaxStaminaFactor = FMath::Clamp(AttackerStats->GetMaxStamina() / 100.f, 0.75f, 2.0f);
		const float MaxManaFactor = FMath::Clamp(AttackerStats->GetMaxMana() / 100.f, 0.75f, 2.5f);
		const float StaminaReadiness = AttackerStats->GetStaminaNormalized();
		const float ManaReadiness = AttackerStats->GetManaNormalized();

		switch (ScalingMode)
		{
		case EKDDamageScalingMode::Physical:
			OffenseMultiplier += ((MaxStaminaFactor - 1.f) * 0.25f) + (StaminaReadiness * 0.10f);
			TryAddDamageTag(OutDamageTags, TEXT("KD.Damage.Channel.Physical"));
			break;
		case EKDDamageScalingMode::Arcane:
			OffenseMultiplier += ((MaxManaFactor - 1.f) * 0.30f) + (ManaReadiness * 0.12f);
			TryAddDamageTag(OutDamageTags, TEXT("KD.Damage.Channel.Arcane"));
			break;
		default:
			OffenseMultiplier += ((MaxStaminaFactor - 1.f) * 0.15f) + ((MaxManaFactor - 1.f) * 0.15f) + ((StaminaReadiness + ManaReadiness) * 0.05f);
			TryAddDamageTag(OutDamageTags, TEXT("KD.Damage.Channel.Hybrid"));
			break;
		}
	}

	float Mitigation = 0.f;
	if (DefenderStats)
	{
		const float MaxHealthFactor = FMath::Clamp(DefenderStats->GetMaxHealth() / 100.f, 0.75f, 3.0f);
		const float StaminaGuard = DefenderStats->GetStaminaNormalized();
		const float ManaGuard = DefenderStats->GetManaNormalized();
		const float HealthGuard = DefenderStats->GetHealthNormalized();

		switch (ScalingMode)
		{
		case EKDDamageScalingMode::Physical:
			Mitigation = ((MaxHealthFactor - 1.f) * 0.12f) + (StaminaGuard * 0.18f) + (HealthGuard * 0.05f);
			break;
		case EKDDamageScalingMode::Arcane:
			Mitigation = ((MaxHealthFactor - 1.f) * 0.10f) + (ManaGuard * 0.18f) + (HealthGuard * 0.05f);
			break;
		default:
			Mitigation = ((MaxHealthFactor - 1.f) * 0.10f) + ((StaminaGuard + ManaGuard) * 0.09f) + (HealthGuard * 0.05f);
			break;
		}
	}

	Mitigation = FMath::Clamp(Mitigation, 0.f, 0.60f);

	const float FinalDamage = RawDamage * FMath::Max(0.10f, OffenseMultiplier) * (1.f - Mitigation);
	return FMath::Max(1.f, FinalDamage);
}

float UKDDamageComponent::CalculateFinalDamage(float RawDamage, AActor* DamagedActor, AController* InstigatedBy,
	AActor* DamageCauser, const FDamageEvent& DamageEvent, const FHitResult& HitInfo,
	bool& bOutIsCritical, FGameplayTagContainer& OutDamageTags) const
{
	// Delegate to the centralized formula to avoid duplicated logic.
	// Adds melee-specific tags (DamageType, BoneHit) on top.
	bOutIsCritical = false;

	const float FinalDamage = CalculateDamageWithTags(RawDamage, DamageCauser, DamagedActor, InstigatedBy, OutDamageTags);

	// Melee-specific tag additions (not relevant for ability path)
	if (DamageEvent.DamageTypeClass)
	{
		TryAddDamageTag(OutDamageTags, TEXT("KD.Damage.Type.Generic"));
	}
	if (!HitInfo.BoneName.IsNone())
	{
		TryAddDamageTag(OutDamageTags, TEXT("KD.Damage.Hit.Bone"));
	}

	return FinalDamage;
}

void UKDDamageComponent::PlayHitReaction(const FKDDamageEvent& DamageEvent)
{
	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
	if (!CharOwner || !CharOwner->GetMesh())
	{
		return;
	}

	UAnimMontage* MontageToPlay = DefaultHitReactionMontage;

	// Determine hit direction and select appropriate montage
	const FVector DamageDir = DamageEvent.DamageDirection.GetSafeNormal();
	const FVector OwnerForward = CharOwner->GetActorForwardVector();

	const float DotProduct = FVector::DotProduct(DamageDir, OwnerForward);

	if (DotProduct > 0.5f)
	{
		// Hit from front
		MontageToPlay = HitReactionFrontMontage ? HitReactionFrontMontage : DefaultHitReactionMontage;
	}
	else if (DotProduct < -0.5f)
	{
		// Hit from back
		MontageToPlay = HitReactionBackMontage ? HitReactionBackMontage : DefaultHitReactionMontage;
	}
	else
	{
		// Hit from side - determine left or right
		const FVector OwnerRight = CharOwner->GetActorRightVector();
		const float RightDot = FVector::DotProduct(DamageDir, OwnerRight);

		if (RightDot > 0.f)
		{
			MontageToPlay = HitReactionRightMontage ? HitReactionRightMontage : DefaultHitReactionMontage;
		}
		else
		{
			MontageToPlay = HitReactionLeftMontage ? HitReactionLeftMontage : DefaultHitReactionMontage;
		}
	}

	if (MontageToPlay)
	{
		UAnimInstance* AnimInstance = CharOwner->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(MontageToPlay);
		}
	}
}

void UKDDamageComponent::ForceHitReaction(UAnimMontage* OverrideMontage)
{
	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
	if (!CharOwner || !CharOwner->GetMesh())
	{
		return;
	}

	UAnimMontage* MontageToPlay = OverrideMontage ? OverrideMontage : DefaultHitReactionMontage.Get();

	if (MontageToPlay)
	{
		UAnimInstance* AnimInstance = CharOwner->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			// EXPLICIT interrupt — only called from status effects, not normal damage
			AnimInstance->Montage_Play(MontageToPlay);
			UE_LOG(LogTemp, Log, TEXT("KDDamageComponent: ForceHitReaction played %s (explicit status effect interrupt)"),
				*MontageToPlay->GetName());
		}
	}
}

void UKDDamageComponent::HandleStatReachedZero()
{
	// Can't die twice
	if (!bIsAlive)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		bIsAlive = false;
	}

	// Broadcast death event
	OnOwnerDeath.Broadcast();

	// TODO: Handle ragdoll if enabled
	if (bEnableRagdollOnDeath)
	{
		ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
		if (CharOwner && CharOwner->GetMesh())
		{
			CharOwner->GetMesh()->SetSimulatePhysics(true);
			CharOwner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
}

void UKDDamageComponent::OnRep_IsAlive()
{
	if (!bIsAlive)
	{
		OnOwnerDeath.Broadcast();
	}
}

void UKDDamageComponent::OnRep_LastDamageReceived()
{
	OnDamageReceived.Broadcast(LastDamageReceived);

	if (LastDamageReceived.DamageDealer)
	{
		UKDDamageComponent* DealerComp = LastDamageReceived.DamageDealer->FindComponentByClass<UKDDamageComponent>();
		if (DealerComp)
		{
			DealerComp->OnDamageInflicted.Broadcast(LastDamageReceived);
		}
	}
}

void UKDDamageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UKDDamageComponent, bIsAlive);
	DOREPLIFETIME(UKDDamageComponent, LastDamageReceived);
}
