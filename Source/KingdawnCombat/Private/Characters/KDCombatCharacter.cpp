// Copyright Kingdawn. All Rights Reserved.

#include "Characters/KDCombatCharacter.h"
#include "Abilities/KDGameplayAbility.h"
#include "Abilities/KDGA_Attack.h"
#include "Abilities/KDGA_ComboAttack.h"
#include "Components/KDAbilitySystemComponent.h"
#include "Components/KDStatsComponent.h"
#include "Components/KDDamageComponent.h"
#include "Components/KDCollisionComponent.h"
#include "Components/KDTargetingComponent.h"
#include "Components/KDFloorMarkerComponent.h"
#include "Components/KDHotbarComponent.h"
#include "Attributes/KDCombatAttributeSet.h"
#include "Core/KDGameplayTags.h"
#include "Data/KDBuildDefinition.h"
#include "Data/KDBuildAbilityResolver.h"
#include "Data/KDHotbarLayoutDefinition.h"
#include "Data/KDWeaponVisualProfile.h"
#include "Mastery/KDMasteryComponent.h"
#include "Mastery/KDMasteryTestBootstrap.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/Skeleton.h"
#include "GameFramework/PlayerController.h"
#include "AIController.h"
#include "BrainComponent.h"

namespace
{
	constexpr float BasicAttackRange = 110.f;
	// Move slightly inside the max range to compensate for nav/path stopping tolerances.
	constexpr float ApproachRangePadding = 50.f;

	FGameplayTag ResolveBasicAttackTag()
	{
		if (FKDGameplayTags::Combat_Action_Attack_Basic.IsValid())
		{
			return FKDGameplayTags::Combat_Action_Attack_Basic;
		}

		return FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Combat.Action.Attack.Basic")), false);
	}

	FGameplayTag ResolveComboLightTag()
	{
		if (FKDGameplayTags::Combat_Skill_Combo_Light.IsValid())
		{
			return FKDGameplayTags::Combat_Skill_Combo_Light;
		}

		return FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Combat.Skill.Combo.Light")), false);
	}

	TSubclassOf<UGameplayAbility> LoadDefaultComboAbilityClass()
	{
		static const FSoftClassPath ComboAbilityPath(TEXT("/Game/KingdawnCombat/Blueprints/BP_GA_ComboAttack.BP_GA_ComboAttack_C"));
		UClass* LoadedClass = ComboAbilityPath.TryLoadClass<UGameplayAbility>();
		return LoadedClass;
	}

	float GetOwnerRangeMultiplier(const AActor* OwnerActor)
	{
		if (!OwnerActor || !OwnerActor->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass()))
		{
			return 1.f;
		}

		const FKDMasteryIdentity OwnerIdentity = IKDEntityInterface::Execute_GetEntityMasteryIdentity(const_cast<AActor*>(OwnerActor));
		return OwnerIdentity.MasteryBranch == EKDMasteryBranch::Wizard ? 1.5f : 1.f;
	}

	bool ResolveAbilityRange(UKDAbilitySystemComponent* ASC, const FGameplayTag& AbilityTag, float& OutMinRange, float& OutMaxRange)
	{
		OutMinRange = 0.f;
		OutMaxRange = BasicAttackRange;

		if (!ASC)
		{
			return !AbilityTag.IsValid() || AbilityTag == ResolveBasicAttackTag();
		}

		if (UKDGameplayAbility* Ability = Cast<UKDGameplayAbility>(ASC->GetAbilityByTag(AbilityTag)))
		{
			Ability->ApplyDefinitionOverrides();
			const float RangeMultiplier = GetOwnerRangeMultiplier(ASC->GetOwnerActor());
			OutMinRange = Ability->MinRange * RangeMultiplier;
			OutMaxRange = Ability->MaxRange * RangeMultiplier;
			return true;
		}

		if (!AbilityTag.IsValid() || AbilityTag == ResolveBasicAttackTag())
		{
			return true;
		}

		return false;
	}


	FVector ComputeApproachLocation(const FVector& SelfLocation, const FVector& TargetLocation, float DesiredMaxRange)
	{
		const float DesiredDistance = FMath::Max(0.f, DesiredMaxRange - ApproachRangePadding);
		FVector Direction = SelfLocation - TargetLocation;
		Direction.Z = 0.f;

		if (Direction.IsNearlyZero())
		{
			Direction = FVector::ForwardVector;
		}
		else
		{
			Direction.Normalize();
		}

		FVector ApproachLocation = TargetLocation + (Direction * DesiredDistance);
		ApproachLocation.Z = SelfLocation.Z;
		return ApproachLocation;
	}
}

AKDCombatCharacter::AKDCombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true; // Always tick for rotation fix
	bAbilitySystemInitialized = false;

	// Auto-approach defaults
	bWantsToAttack = false;
	bAutoRepeatBasicAttack = false;
	QueuedAttackTarget = nullptr;
	ApproachAcceptanceRadius = 50.f;
	bHasMovementTarget = false;
	MovementTargetLocation = FVector::ZeroVector;
	MovementFacingInterpSpeed = 10.f;
	MovementFacingTurnRate = 720.f;
	MovementFacingMinSpeed = 5.f;
	MovementTargetReachedThreshold = 25.f;
	bAutoAlignMeshYaw = true;
	MeshYawOffset = -90.f;

	// Create components
	AbilitySystemComp = CreateDefaultSubobject<UKDAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	StatsComp = CreateDefaultSubobject<UKDStatsComponent>(TEXT("StatsComp"));
	DamageComp = CreateDefaultSubobject<UKDDamageComponent>(TEXT("DamageComp"));
	CollisionComp = CreateDefaultSubobject<UKDCollisionComponent>(TEXT("CollisionComp"));
	TargetingComp = CreateDefaultSubobject<UKDTargetingComponent>(TEXT("TargetingComp"));
	FloorMarkerComp = CreateDefaultSubobject<UKDFloorMarkerComponent>(TEXT("FloorMarkerComp"));
	MasteryComp = CreateDefaultSubobject<UKDMasteryComponent>(TEXT("MasteryComp"));

	// Weapon visual mesh ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â attached to character hand socket
	WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComp"));
	WeaponMeshComp->SetupAttachment(GetMesh(), FName(TEXT("hand_r")));
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComp->SetGenerateOverlapEvents(false);
	WeaponMeshComp->bCastDynamicShadow = false;
	WeaponMeshComp->bAffectDynamicIndirectLighting = false;

	MainHandWeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MainHandWeaponMeshComp"));
	MainHandWeaponMeshComp->SetupAttachment(GetMesh(), FName(TEXT("hand_r")));
	MainHandWeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MainHandWeaponMeshComp->SetGenerateOverlapEvents(false);
	MainHandWeaponMeshComp->bCastDynamicShadow = false;
	MainHandWeaponMeshComp->bAffectDynamicIndirectLighting = false;

	OffHandWeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("OffHandWeaponMeshComp"));
	OffHandWeaponMeshComp->SetupAttachment(GetMesh(), FName(TEXT("hand_l")));
	OffHandWeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OffHandWeaponMeshComp->SetGenerateOverlapEvents(false);
	OffHandWeaponMeshComp->bCastDynamicShadow = false;
	OffHandWeaponMeshComp->bAffectDynamicIndirectLighting = false;

	StatusDebugTextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDebugTextComp"));
	StatusDebugTextComp->SetupAttachment(GetRootComponent());
	StatusDebugTextComp->SetHorizontalAlignment(EHTA_Center);
	StatusDebugTextComp->SetWorldSize(28.f);
	StatusDebugTextComp->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	StatusDebugTextComp->SetTextRenderColor(FColor(80, 170, 255));
	StatusDebugTextComp->SetHiddenInGame(true);
	StatusDebugTextComp->SetVisibility(false);
	StatusDebugTextComp->SetText(FText::GetEmpty());

	// Initialize weapon attachment profiles for each weapon family.
	// These define socket, offsets, and reference fallback for weapon visual placement.
	static const FSoftObjectPath WitchReferenceMeshPath(TEXT("/Game/Witch_Animation_Set/Animation/Skeleton/SK_Mannequin.SK_Mannequin"));
	static const FSoftObjectPath WitchReferenceSkeletonPath(TEXT("/Game/Witch_Animation_Set/Animation/Skeleton/S_Mannequin_Skeleton.S_Mannequin_Skeleton"));

	// Staff: Right hand socket, use Witch reference socket as baseline
	WeaponProfiles.Add(EKDWeaponFamily::Staff, FKDWeaponAttachProfile(
		FName(TEXT("hand_r")),
		FName(TEXT("hand_rSocket")),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		WitchReferenceMeshPath,
		FName(TEXT("hand_rSocket"))
	));

	// OneHandedSword: attach to weapon_rsocket
	WeaponProfiles.Add(EKDWeaponFamily::OneHandedSword, FKDWeaponAttachProfile(
		FName(TEXT("hand_r")),
		FName(TEXT("weapon_rsocket")),  // Right hand weapon socket
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		WitchReferenceSkeletonPath,
		FName(TEXT("weapon_rSocket"))
	));

	// Shield: attach to shield_Lsocket
	WeaponProfiles.Add(EKDWeaponFamily::Shield, FKDWeaponAttachProfile(
		FName(TEXT("lowerarm_l")),
		FName(TEXT("shield_Lsocket")),  // Left hand shield socket (uppercase L)
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		WitchReferenceSkeletonPath,
		FName(TEXT("shield_lSocket"))
	));

	// TwoHandedSword: shares the same right-hand reference family.
	WeaponProfiles.Add(EKDWeaponFamily::TwoHandedSword, FKDWeaponAttachProfile(
		FName(TEXT("hand_r")),
		FName(TEXT("hand_rSocket")),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		WitchReferenceMeshPath,
		FName(TEXT("hand_rSocket"))
	));

	// Bow: Both hands (attach to right for now, could be spine/center)
	WeaponProfiles.Add(EKDWeaponFamily::Bow, FKDWeaponAttachProfile(
		FName(TEXT("hand_r")),
		FName(TEXT("hand_rSocket")),
		FVector::ZeroVector,
		FRotator(0.f, 90.f, 0.f),                  // Bow faces sideways
		FVector::OneVector,
		WitchReferenceMeshPath,
		FName(TEXT("hand_rSocket"))
	));

	CombatClass = EKDCombatClass::Warrior;
	// Default basic attack tag (mouse double-click)
	AttackAbilityTag = FKDGameplayTags::Combat_Action_Attack_Basic;
	CombatTeamTag = FKDGameplayTags::Team_Player;
	CombatBranch = EKDCombatBranch::None;
	EquippedWeapon = EKDWeaponType::OneHandedSword;

	// Movement settings: character owns facing explicitly in Tick.
	// This avoids camera yaw and SimpleMove path-following fighting over pawn rotation.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator::ZeroRotator;
	}
}

UAbilitySystemComponent* AKDCombatCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

FGameplayTag AKDCombatCharacter::GetEntityCombatTeam_Implementation() const
{
	return CombatTeamTag;
}

// --- Targeting ---

AActor* AKDCombatCharacter::GetCurrentTarget() const
{
	return TargetingComp ? TargetingComp->GetCurrentTarget() : nullptr;
}

void AKDCombatCharacter::SetCombatTarget(AActor* NewTarget)
{
	if (TargetingComp)
	{
		TargetingComp->SetTarget(NewTarget);
	}
}

void AKDCombatCharacter::ClearCombatTarget()
{
	if (TargetingComp)
	{
		TargetingComp->ClearTarget();
	}
}

// --- Floor Marker ---

void AKDCombatCharacter::SpawnFloorMarker(const FVector& WorldLocation)
{
	if (FloorMarkerComp)
	{
		FloorMarkerComp->SpawnMarkerAtLocation(WorldLocation);
	}
}

// --- Movement Target ---

void AKDCombatCharacter::SetMovementTarget(const FVector& TargetLocation)
{
	MovementTargetLocation = TargetLocation;
	bHasMovementTarget = true;
}

void AKDCombatCharacter::ClearMovementTarget()
{
	bHasMovementTarget = false;
	MovementTargetLocation = FVector::ZeroVector;
}

// --- Combat Input ---

void AKDCombatCharacter::OnAttackInputPressed()
{
	TryAttack();
}

void AKDCombatCharacter::OnCycleTargetInputPressed()
{
	if (TargetingComp)
	{
		TargetingComp->CycleTarget(true);
	}
}

void AKDCombatCharacter::OnClearTargetInputPressed()
{
	ClearCombatTarget();
}

bool AKDCombatCharacter::TryAttack()
{
	// Require a valid target for attacks
	if (!TargetingComp || !TargetingComp->IsCurrentTargetValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: Cannot attack - no valid target"));
		return false;
	}

	// Force AttackAbilityTag to the correct basic attack tag.
	// Blueprint CDO may carry stale tags like KD.Combat.Action.Attack.Light.
	// Code owns this identity - always normalize to the correct tag.
	AttackAbilityTag = ResolveBasicAttackTag();

	if (!AbilitySystemComp || !AttackAbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Cannot attack - ASC valid=%d, AttackAbilityTag valid=%d, Tag=%s"),
			AbilitySystemComp != nullptr, AttackAbilityTag.IsValid(), *AttackAbilityTag.ToString());
		return false;
	}

	if (AbilitySystemComp->IsPerformingAbility())
	{
		if (AbilitySystemComp->IsInAbilityState(AttackAbilityTag))
		{
			if (UKDGA_ComboAttack* ActiveComboAbility = Cast<UKDGA_ComboAttack>(AbilitySystemComp->GetAbilityByTag(AttackAbilityTag)))
			{
				ActiveComboAbility->SendComboInput();
				UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: TryAttack routed to SendComboInput for active combo"));
				return true;
			}
		}

		return false;
	}

	AActor* CurrentTarget = TargetingComp->GetCurrentTarget();
	const float DistanceToTarget = CurrentTarget ? FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) : -1.f;
	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: TryAttack -> Tag=%s Target=%s Distance=%.1f"),
		*AttackAbilityTag.ToString(),
		CurrentTarget ? *CurrentTarget->GetName() : TEXT("None"),
		DistanceToTarget);

	const bool bTriggered = AbilitySystemComp->TriggerAbilityByTag(AttackAbilityTag);
	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: TriggerAbilityByTag returned %d"), bTriggered);
	return bTriggered;
}

bool AKDCombatCharacter::ActivateAbilityByTagWithAutoApproach(FGameplayTag AbilityTag)
{
	if (!AbilitySystemComp || !AbilityTag.IsValid())
	{
		return false;
	}

	const FGameplayTag BasicAttackTag = ResolveBasicAttackTag();
	if (AbilityTag != BasicAttackTag)
	{
		// Skills always take priority over the sustained basic attack filler loop.
		StopBasicAttackAutoRepeat();

		if (AbilitySystemComp->IsPerformingAbility() && AbilitySystemComp->IsInAbilityState(BasicAttackTag))
		{
			const FGameplayAbilitySpecHandle BasicHandle = AbilitySystemComp->GetAbilityHandleByTag(BasicAttackTag);
			if (BasicHandle.IsValid())
			{
				AbilitySystemComp->CancelAbilityHandle(BasicHandle);
			}
		}
	}

	UGameplayAbility* Ability = AbilitySystemComp->GetAbilityByTag(AbilityTag);
	if (!Ability && ActiveBuildDefinition)
	{
		if (TSubclassOf<UGameplayAbility> MissingAbilityClass = UKDBuildAbilityResolver::ResolveAbilityClass(AbilityTag))
		{
			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Auto-granting missing build ability %s for tag %s"),
				*GetNameSafe(MissingAbilityClass), *AbilityTag.ToString());
			AbilitySystemComp->GrantAbility(MissingAbilityClass, AbilityTag);
			Ability = AbilitySystemComp->GetAbilityByTag(AbilityTag);
		}
	}

	UKDGameplayAbility* AbilityConfig = Cast<UKDGameplayAbility>(Ability);
	if (!AbilityConfig)
	{
		return AbilitySystemComp->TriggerAbilityByTag(AbilityTag);
	}

	AbilityConfig->ApplyDefinitionOverrides();

	if (!AbilityConfig->bRequiresTarget)
	{
		return AbilitySystemComp->TriggerAbilityByTag(AbilityTag);
	}

	if (!TargetingComp || !TargetingComp->IsCurrentTargetValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: Cannot activate %s - no valid target"), *AbilityTag.ToString());
		return false;
	}

	AActor* Target = TargetingComp->GetCurrentTarget();
	const float Distance = Target ? FVector::Dist(GetActorLocation(), Target->GetActorLocation()) : TNumericLimits<float>::Max();
	if (Target && Distance <= AbilityConfig->MaxRange && Distance >= AbilityConfig->MinRange)
	{
		return AbilitySystemComp->TriggerAbilityByTag(AbilityTag);
	}

	bWantsToAttack = true;
	QueuedAttackTarget = Target;
	QueuedAbilityTag = AbilityTag;
	const FVector ApproachLocation = ComputeApproachLocation(GetActorLocation(), Target->GetActorLocation(), AbilityConfig->MaxRange);
	SetMovementTarget(ApproachLocation);

	if (AController* MyController = GetController())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, ApproachLocation);
	}

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Queued ability %s on approach (distance %.1f, max range %.1f, move target %s)"),
		*AbilityTag.ToString(), Distance, AbilityConfig->MaxRange, *ApproachLocation.ToCompactString());
	return true;
}

// --- Auto-Approach (Double-Click Attack) ---

void AKDCombatCharacter::QueueAttackOnApproach(AActor* Target)
{
	if (!Target || !TargetingComp)
	{
		return;
	}

	// BUG FIX: Cancel any previous approach before starting a new one
	// This prevents stacking multiple queued attacks from repeated double-clicks
	if (bWantsToAttack)
	{
		CancelQueuedAttack();
	}

	// Set target (idempotent ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â safe to call even if already targeted)
	SetCombatTarget(Target);

	// Check if already in range using the granted basic attack definition.
	float AttackMinRange = 0.f;
	float AttackRange = BasicAttackRange;
	ResolveAbilityRange(AbilitySystemComp, ResolveBasicAttackTag(), AttackMinRange, AttackRange);
	const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

	if (Distance <= AttackRange && Distance >= AttackMinRange)
	{
		// Already in range ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â attack immediately and start auto-repeat
		if (TryAttack())
		{
			StartBasicAttackAutoRepeat();
		}
		return;
	}

	// Out of range ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â queue auto-approach toward the farthest valid attack distance.
	bWantsToAttack = true;
	QueuedAttackTarget = Target;
	QueuedAbilityTag = ResolveBasicAttackTag();
	const FVector ApproachLocation = ComputeApproachLocation(GetActorLocation(), Target->GetActorLocation(), AttackRange);
	SetMovementTarget(ApproachLocation);

	if (AController* MyController = GetController())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, ApproachLocation);
	}

	UE_LOG(LogTemp, Log, TEXT("KDCombat: Queued attack on %s (distance: %.1f, range: %.1f, move target %s)"),
		*Target->GetName(), Distance, AttackRange, *ApproachLocation.ToCompactString());
}

void AKDCombatCharacter::CancelQueuedAttack()
{
	if (!bWantsToAttack && !bAutoRepeatBasicAttack)
	{
		return; // Nothing to cancel
	}

	bWantsToAttack = false;
	QueuedAttackTarget = nullptr;
	QueuedAbilityTag = FGameplayTag();
	ClearMovementTarget();

	// Stop auto-repeat when canceling queued attack (ground click cancels auto-repeat)
	StopBasicAttackAutoRepeat();

	// BUG FIX: Stop any ongoing AI movement
	if (AController* MyController = GetController())
	{
		MyController->StopMovement();
	}

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Cancelled queued attack and auto-repeat"));
}

// --- Auto-Repeat Basic Attack (Silkroad-style) ---

void AKDCombatCharacter::StartBasicAttackAutoRepeat()
{
	if (bAutoRepeatBasicAttack)
	{
		return; // Already active
	}

	bAutoRepeatBasicAttack = true;
	UE_LOG(LogTemp, Log, TEXT("KDCombat: Started basic attack auto-repeat"));
}

void AKDCombatCharacter::StopBasicAttackAutoRepeat()
{
	if (!bAutoRepeatBasicAttack)
	{
		return; // Already stopped
	}

	bAutoRepeatBasicAttack = false;
	UE_LOG(LogTemp, Log, TEXT("KDCombat: Stopped basic attack auto-repeat"));
}

void AKDCombatCharacter::TryAutoRepeatAttack()
{
	// Validate auto-repeat is active
	if (!bAutoRepeatBasicAttack)
	{
		return;
	}

	// Validate target is still valid
	if (!TargetingComp || !TargetingComp->IsCurrentTargetValid())
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombat: Auto-repeat stopped - target lost"));
		StopBasicAttackAutoRepeat();
		return;
	}

	// Validate target is in range
	AActor* CurrentTarget = TargetingComp->GetCurrentTarget();
	if (!CurrentTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombat: Auto-repeat stopped - no target"));
		StopBasicAttackAutoRepeat();
		return;
	}

	float AttackMinRange = 0.f;
	float AttackRange = BasicAttackRange;
	ResolveAbilityRange(AbilitySystemComp, ResolveBasicAttackTag(), AttackMinRange, AttackRange);
	const float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > AttackRange || Distance < AttackMinRange)
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombat: Auto-repeat re-approach - target outside basic range (%.1f not in %.1f-%.1f)"), Distance, AttackMinRange, AttackRange);
		QueueAttackOnApproach(CurrentTarget);
		return;
	}

	// Try to execute next attack
	if (!TryAttack())
	{
		// Attack failed (likely insufficient stamina or ability on cooldown)
		UE_LOG(LogTemp, Log, TEXT("KDCombat: Auto-repeat stopped - attack failed (likely insufficient resources)"));
		StopBasicAttackAutoRepeat();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombat: Auto-repeat continuing - next attack queued"));
	}
}

// --- Team ---

void AKDCombatCharacter::HandleDeath()
{
	// Stop auto-repeat
	StopBasicAttackAutoRepeat();
	CancelQueuedAttack();

	// Disable movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	// Disable collision on the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Clear target
	ClearCombatTarget();

	// Stop AI BehaviorTree if this character is AI-controlled (enemy NPC).
	// Prevents the BT from continuing to tick after death (e.g., trying to move or attack).
	if (AAIController* AICtrl = Cast<AAIController>(GetController()))
	{
		AICtrl->StopMovement();
		if (UBrainComponent* Brain = AICtrl->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Death"));
		}
	}
}

bool AKDCombatCharacter::IsDead() const
{
	return DamageComp ? !DamageComp->GetIsAlive() : false;
}

// --- Lifecycle ---

void AKDCombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	// CRITICAL: Force tick enabled ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â Blueprint CDO may disable it
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);

	// FIX: If character is floating above ground, snap to ground
	// This can happen if the Blueprint has incorrect Z placement
	FVector CurrentLoc = GetActorLocation();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float ExpectedGroundZ = CapsuleHalfHeight; // Character should stand at this Z
	
	// Always check and snap to ground - threshold lowered to catch more cases
	if (FMath::Abs(CurrentLoc.Z - ExpectedGroundZ) > 1.f) // More than 1 unit off
	{
		// Trace down to find ground
		FHitResult HitResult;
		FVector TraceStart = CurrentLoc + FVector(0.f, 0.f, 500.f);
		FVector TraceEnd = CurrentLoc - FVector(0.f, 0.f, 1000.f);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			float GroundZ = HitResult.ImpactPoint.Z + CapsuleHalfHeight;
			if (FMath::Abs(CurrentLoc.Z - GroundZ) > 1.f)
			{
				FVector NewLoc = CurrentLoc;
				NewLoc.Z = GroundZ;
				SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);
				UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Snapped to ground Z=%.1f -> Z=%.1f"), CurrentLoc.Z, GroundZ);
			}
		}
		else
		{
			// No ground found below ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â force to expected ground Z
			UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: No ground found below, forcing Z=%.1f (was Z=%.1f)"), ExpectedGroundZ, CurrentLoc.Z);
			FVector NewLoc = CurrentLoc;
			NewLoc.Z = ExpectedGroundZ;
			SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: BeginPlay - Tick enabled=%d, CanEverTick=%d, Pos=(%.1f, %.1f, %.1f)"),
		PrimaryActorTick.IsTickFunctionEnabled(), PrimaryActorTick.bCanEverTick,
		GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);


	// Build-driven setup: CVar override takes precedence, then Blueprint default.
	// kd.build.testoverride: 0=Blueprint, 1=BladeShield, 2=Wizard
	if (UKDBuildDefinition* OverrideBuild = GetTestBuildOverride())
	{
		ApplyBuildDefinition(OverrideBuild);
		UE_LOG(LogTemp, Display, TEXT("KDCombatCharacter: Applied CVar test build override: %s"), *OverrideBuild->GetName());
	}
	else if (SelectedBuildDefinition)
	{
		ApplyBuildDefinition(SelectedBuildDefinition);
	}

	// Ensure AttackAbilityTag is always set to the basic attack tag.
	// Code owns this identity — never rely on Blueprint CDO defaults.
	AttackAbilityTag = ResolveBasicAttackTag();

	// ROTATION FIX: Force movement settings at BeginPlay to override any Blueprint defaults.
	// Movement-facing is handled manually in Tick so camera orbit cannot make the pawn backpedal.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->RotationRate = FRotator::ZeroRotator;

		UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Rotation settings applied bOrientRotationToMovement=%d, bUseControllerDesiredRotation=%d, bUseControllerRotationYaw=%d"),
			MoveComp->bOrientRotationToMovement,
			MoveComp->bUseControllerDesiredRotation,
			bUseControllerRotationYaw);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const FRotator MeshRelativeRotation = MeshComp->GetRelativeRotation();
		if (bAutoAlignMeshYaw && FMath::IsNearlyZero(MeshRelativeRotation.Yaw, 1.f))
		{
			FRotator NewMeshRotation = MeshRelativeRotation;
			NewMeshRotation.Yaw = MeshYawOffset;
			MeshComp->SetRelativeRotation(NewMeshRotation);

			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Auto-aligned mesh yaw from %.1f to %.1f"),
				MeshRelativeRotation.Yaw, NewMeshRotation.Yaw);
		}
	}

	// Auto-initialize collision component with character mesh
	if (CollisionComp && GetMesh())
	{
		TArray<TEnumAsByte<ECollisionChannel>> DefaultChannels;
		DefaultChannels.Add(ECC_Pawn);
		CollisionComp->SetupCollisionManager(GetMesh(), DefaultChannels);
		CollisionComp->SetActorOwner(this);
	}

	InitializeAbilitySystem();

	// Apply build-driven hotbar layout if present
	if (ActiveBuildDefinition && ActiveBuildDefinition->DefaultHotbarLayout.IsValid())
	{
		UKDHotbarLayoutDefinition* Layout = ActiveBuildDefinition->DefaultHotbarLayout.LoadSynchronous();
		if (Layout && IsPlayerControlled())
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				if (UKDHotbarComponent* Hotbar = PC->FindComponentByClass<UKDHotbarComponent>())
				{
					Hotbar->ClearAllSlots();
					if (Hotbar->SetupSlotsFromLayout(Layout))
					{
						UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Applied build-driven hotbar layout '%s'"),
							*Layout->GetName());
					}
				}
			}
		}
	}

	// Equip visual weapon based on initial EquippedWeapon state
	UpdateWeaponMesh();

	// Bind ability lifecycle events for auto-repeat
	if (AbilitySystemComp)
	{
		AbilitySystemComp->OnAbilityStartedEvent.AddDynamic(this, &AKDCombatCharacter::OnAbilityStarted);
		AbilitySystemComp->OnAbilityFinishedEvent.AddDynamic(this, &AKDCombatCharacter::OnAbilityEnded);
	}

	// Bind death event
	if (DamageComp)
	{
		DamageComp->OnOwnerDeath.AddDynamic(this, &AKDCombatCharacter::HandleDeath);
	}

	// Mastery test bootstrap: when kd.mastery.testmode == 1,
	// loads the authored mastery definition, activates mastery,
	// unlocks skills, and reconciles ASC + hotbar.
	FKDMasteryTestBootstrap::RunTestBootstrap(this);
}

void AKDCombatCharacter::OnAbilityEnded(FGameplayTag AbilityTag)
{
	// Only trigger auto-repeat for basic attack ability
	const FGameplayTag BasicAttackTag = ResolveBasicAttackTag();
	if (BasicAttackTag.IsValid() && AbilityTag == BasicAttackTag)
	{
		// Basic attack ended ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â check if we should continue auto-repeat
		if (bAutoRepeatBasicAttack)
		{
			UE_LOG(LogTemp, Log, TEXT("KDCombat: Basic attack ended, deferring auto-repeat to next frame"));
			
			// CRITICAL FIX: Defer auto-repeat to next frame to avoid same-call-stack race condition.
			// If we call TryAutoRepeatAttack() immediately here (inside NotifyAbilityEnded),
			// it triggers TryAttack() while the first ability instance hasn't fully cleaned up.
			// This violates InstancedPerActor policy causing the GAS ensure:
			//   "Default__BP_GA_Attack_C was still active"
			// and the montage playback failure:
			//   "UAbilityTask_PlayMontageAndWait failed to play montage AM_Attack_01"
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AKDCombatCharacter::TryAutoRepeatAttack);
		}
	}
}

void AKDCombatCharacter::OnAbilityStarted(FGameplayTag AbilityTag)
{
	// If a non-basic-attack ability starts, stop auto-repeat.
	// Hotbar skills (combo, etc.) take priority over auto-repeat.
	const FGameplayTag BasicAttackTag = ResolveBasicAttackTag();
	if (bAutoRepeatBasicAttack && BasicAttackTag.IsValid() && AbilityTag != BasicAttackTag)
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombat: Auto-repeat stopped - ability %s started (priority override)"), *AbilityTag.ToString());
		StopBasicAttackAutoRepeat();
	}
}

void AKDCombatCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Re-init GAS on possession (important for player characters)
	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);
	}
}

void AKDCombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMovementFacing(DeltaTime);
	UpdateStatusDebugIndicator();

	if (bHasMovementTarget && !bWantsToAttack)
	{
		const FVector ToMoveTarget = MovementTargetLocation - GetActorLocation();
		const FVector ToMoveTarget2D(ToMoveTarget.X, ToMoveTarget.Y, 0.f);
		const FVector Velocity2D(GetVelocity().X, GetVelocity().Y, 0.f);
		if (ToMoveTarget2D.SizeSquared() <= FMath::Square(MovementTargetReachedThreshold) &&
			Velocity2D.SizeSquared() <= FMath::Square(MovementFacingMinSpeed))
		{
			ClearMovementTarget();
		}
	}

		// Auto-approach tick: check if we're in the queued ability's valid range yet
	if (bWantsToAttack && QueuedAttackTarget.IsValid())
	{
		AActor* Target = QueuedAttackTarget.Get();
		const FGameplayTag AbilityToFire = QueuedAbilityTag.IsValid() ? QueuedAbilityTag : ResolveBasicAttackTag();
		float MinRange = 0.f;
		float MaxRange = BasicAttackRange;
		ResolveAbilityRange(AbilitySystemComp, AbilityToFire, MinRange, MaxRange);

		MovementTargetLocation = ComputeApproachLocation(GetActorLocation(), Target->GetActorLocation(), MaxRange);
		bHasMovementTarget = true;
		const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

		if (Distance <= MaxRange && Distance >= MinRange)
		{
			bWantsToAttack = false;
			QueuedAttackTarget = nullptr;
			QueuedAbilityTag = FGameplayTag();

			if (AController* MyController = GetController())
			{
				MyController->StopMovement();
			}

			ClearMovementTarget();

			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Auto-approach reached range -> activate %s at distance %.1f (min %.1f, max %.1f)"),
				*AbilityToFire.ToString(), Distance, MinRange, MaxRange);

			const bool bActivated = (AbilityToFire == ResolveBasicAttackTag())
				? TryAttack()
				: AbilitySystemComp->TriggerAbilityByTag(AbilityToFire);

			if (bActivated && AbilityToFire == ResolveBasicAttackTag())
			{
				StartBasicAttackAutoRepeat();
			}
			else if (!bActivated)
			{
				// Activation failed (e.g., insufficient mana, cooldown).
				// Clear the queued ability so the next hotbar press starts fresh
				// instead of re-triggering the approach-activate loop.
				QueuedAbilityTag = FGameplayTag();
				bWantsToAttack = false;
				QueuedAttackTarget = nullptr;
				ClearMovementTarget();
			}
		}
	}
	else if (bWantsToAttack && !QueuedAttackTarget.IsValid())
	{
		// Target was lost (died, destroyed, etc.) ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â cancel approach
		CancelQueuedAttack();
	}
}

void AKDCombatCharacter::UpdateMovementFacing(float DeltaTime)
{
	if (!GetCharacterMovement())
	{
		return;
	}

	if (AbilitySystemComp && AbilitySystemComp->IsPerformingAbility() && !bWantsToAttack)
	{
		return;
	}

	const FVector DesiredDirection = GetDesiredMovementFacingDirection();
	if (DesiredDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator DesiredRotation = FRotator(0.f, DesiredDirection.Rotation().Yaw, 0.f);
	const FRotator NewRotation = MovementFacingTurnRate > 0.f
		? FMath::RInterpConstantTo(CurrentRotation, DesiredRotation, DeltaTime, MovementFacingTurnRate)
		: FMath::RInterpTo(CurrentRotation, DesiredRotation, DeltaTime, MovementFacingInterpSpeed);
	SetActorRotation(NewRotation);
}

FVector AKDCombatCharacter::GetDesiredMovementFacingDirection() const
{
	const FVector CurrentVelocity = GetVelocity();
	const FVector Velocity2D(CurrentVelocity.X, CurrentVelocity.Y, 0.f);
	if (Velocity2D.SizeSquared() > FMath::Square(MovementFacingMinSpeed))
	{
		return Velocity2D.GetSafeNormal();
	}

	if (bWantsToAttack && QueuedAttackTarget.IsValid())
	{
		const FVector ToTarget = QueuedAttackTarget->GetActorLocation() - GetActorLocation();
		const FVector ToTarget2D(ToTarget.X, ToTarget.Y, 0.f);
		if (!ToTarget2D.IsNearlyZero())
		{
			return ToTarget2D.GetSafeNormal();
		}
	}

	if (bHasMovementTarget)
	{
		const FVector ToMoveTarget = MovementTargetLocation - GetActorLocation();
		const FVector ToMoveTarget2D(ToMoveTarget.X, ToMoveTarget.Y, 0.f);
		if (ToMoveTarget2D.SizeSquared() <= FMath::Square(MovementTargetReachedThreshold))
		{
			return FVector::ZeroVector;
		}

		return ToMoveTarget2D.GetSafeNormal();
	}

	return FVector::ZeroVector;
}

void AKDCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input is now handled by KDPlayerController, not directly on the character.
	// Tab/Escape/Attack are all routed through KDPlayerController ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬ÂÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ KDClickInputHandler.
	// Character methods (OnCycleTargetInputPressed, OnClearTargetInputPressed, TryAttack)
	// are called by the controller, not bound directly here.
}

float AKDCombatCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageComp)
	{
		return DamageComp->TakeDamage(this, DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AKDCombatCharacter::InitializeAbilitySystem()
{
	if (bAbilitySystemInitialized)
	{
		return;
	}

	if (AbilitySystemComp)
	{
		// Init GAS actor info
		AbilitySystemComp->InitAbilityActorInfo(this, this);

		// Create and add the combat attribute set
		// NOTE: Outer must be 'this' (the actor), NOT AbilitySystemComp.
		// The ATTRIBUTE_ACCESSORS Set* macros call GetOwningActor() which
		// walks the outer chain expecting an AActor. If outer is a UActorComponent
		// the Cast<Actor> fails with a fatal error.
		CombatAttributes = NewObject<UKDCombatAttributeSet>(this);
		AbilitySystemComp->AddAttributeSetSubobject(CombatAttributes.Get());

		// Default values are already set in UKDCombatAttributeSet constructor
		// via InitHealth(100), InitMaxHealth(100), etc.
		// If you need to override defaults, use Init* (not Set*) to avoid
		// the actor-cast path before the ASC is fully initialized:
		// CombatAttributes->InitHealth(100.f);
		// CombatAttributes->InitMaxHealth(100.f);

		// Build-driven ability grant takes precedence over legacy branch-based grant
		TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;
		TArray<FGameplayTag> TagsToGrant;

		if (ActiveBuildDefinition)
		{
			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Build-driven ability grant from '%s'"), *ActiveBuildDefinition->GetName());

			// Grant BasicAttackAbility (mandatory)
			if (ActiveBuildDefinition->BasicAttackAbility.IsValid())
			{
				TSubclassOf<UGameplayAbility> BasicClass = UKDBuildAbilityResolver::ResolveAbilityClass(
					ActiveBuildDefinition->BasicAttackAbility);
				if (BasicClass)
				{
					AbilitiesToGrant.Add(BasicClass);
					TagsToGrant.Add(ActiveBuildDefinition->BasicAttackAbility);
					UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Resolved BasicAttack %s -> %s"),
						*ActiveBuildDefinition->BasicAttackAbility.ToString(), *BasicClass->GetName());
				}
			}

			// Grant ComboAbility (mandatory)
			if (ActiveBuildDefinition->ComboAbility.IsValid())
			{
				TSubclassOf<UGameplayAbility> ComboClass = UKDBuildAbilityResolver::ResolveAbilityClass(
					ActiveBuildDefinition->ComboAbility);
				if (ComboClass)
				{
					AbilitiesToGrant.Add(ComboClass);
					TagsToGrant.Add(ActiveBuildDefinition->ComboAbility);
					UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Resolved Combo %s -> %s"),
						*ActiveBuildDefinition->ComboAbility.ToString(), *ComboClass->GetName());
				}
			}

			// Grant additional default abilities (avoid duplicates)
			for (const FGameplayTag& AbilityTag : ActiveBuildDefinition->DefaultAbilities)
			{
				if (!AbilityTag.IsValid())
				{
					continue;
				}

				// Skip if already granted as BasicAttack or Combo
				if (TagsToGrant.Contains(AbilityTag))
				{
					continue;
				}

				TSubclassOf<UGameplayAbility> AbilityClass = UKDBuildAbilityResolver::ResolveAbilityClass(AbilityTag);
				if (AbilityClass)
				{
					AbilitiesToGrant.Add(AbilityClass);
					TagsToGrant.Add(AbilityTag);
					UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Resolved DefaultAbility %s -> %s"),
						*AbilityTag.ToString(), *AbilityClass->GetName());
				}
			}

			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Build-driven grant complete - %d abilities, %d tags"),
				AbilitiesToGrant.Num(), TagsToGrant.Num());
		}
		// Legacy fallback: branch-based hardcoded grant
		else
		{
			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Legacy fallback - branch-based ability grant for %s"),
				*UEnum::GetValueAsString(MasteryIdentity.MasteryBranch));

			AbilitiesToGrant = DefaultAbilities;
			if (MasteryIdentity.MasteryBranch == EKDMasteryBranch::BladeShield)
			{
				AbilitiesToGrant.Empty();
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Abilities/BP_GA_Attack.BP_GA_Attack_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/BP_GA_ComboAttack.BP_GA_ComboAttack_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/BP_GA_BladeShield_HeavyStrike.BP_GA_BladeShield_HeavyStrike_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/BP_GA_BladeShield_ShieldBash.BP_GA_BladeShield_ShieldBash_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
			}
			else if (MasteryIdentity.MasteryBranch == EKDMasteryBranch::Wizard)
			{
				AbilitiesToGrant.Empty();
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_BasicAttack.BP_GA_Wizard_BasicAttack_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_ArcaneBolt.BP_GA_Wizard_ArcaneBolt_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_Fireball.BP_GA_Wizard_Fireball_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_FrostNova.BP_GA_Wizard_FrostNova_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
				if (UClass* C = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_ManaShield.BP_GA_Wizard_ManaShield_C")).TryLoadClass<UGameplayAbility>()) AbilitiesToGrant.Add(C);
			}

			// Auto-generate tags for legacy grants (class names only, tags resolved at grant time)
			TagsToGrant.Empty();
		}
		// Grant abilities (build-driven or legacy)
		for (int32 Idx = 0; Idx < AbilitiesToGrant.Num(); ++Idx)
		{
			const TSubclassOf<UGameplayAbility>& AbilityClass = AbilitiesToGrant[Idx];
			if (!AbilityClass)
			{
				continue;
			}

			// If we have a pre-resolved tag from build-driven grant, use it.
			// Otherwise, extract tag from ability CDO (legacy fallback path).
			FGameplayTag TagToUse;
			if (TagsToGrant.IsValidIndex(Idx) && TagsToGrant[Idx].IsValid())
			{
				TagToUse = TagsToGrant[Idx];
			}
			else
			{
				const FGameplayTag LegacyLightAttackTag = FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Combat.Action.Attack.Light")), false);

				// Extract tag from the ability default object first. Blueprint children may leave
				// AbilityTag as None even when their native parent or AbilityDefinition provides it,
				// so walk up the class hierarchy until a valid tag is found.
				for (UClass* SearchClass = AbilityClass.Get(); SearchClass && !TagToUse.IsValid(); SearchClass = SearchClass->GetSuperClass())
				{
					if (SearchClass->IsChildOf<UKDGameplayAbility>())
					{
						if (UKDGameplayAbility* AbilityCDO = Cast<UKDGameplayAbility>(SearchClass->GetDefaultObject()))
						{
							AbilityCDO->ApplyDefinitionOverrides();
							TagToUse = AbilityCDO->AbilityTag;
						}
					}
					else if (!SearchClass->IsChildOf<UGameplayAbility>())
					{
						break;
					}
				}

				// Normalize only the exact shared basic shell. Custom KDGA_Attack subclasses
				// like HeavyStrike or WizardBasic must keep their own definition-driven tags.
				const FString AbilityPath = AbilityClass->GetPathName();
				const bool bIsSharedBasicAttackShell =
					AbilityPath.Contains(TEXT("/Game/KingdawnCombat/Abilities/BP_GA_Attack.")) ||
					AbilityClass->GetName().Equals(TEXT("BP_GA_Attack_C"));

				if (bIsSharedBasicAttackShell)
				{
					TagToUse = ResolveBasicAttackTag();
					UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: FORCED shared basic attack shell -> %s"),
						*TagToUse.ToString());
				}
				else if (AbilityClass->IsChildOf<UKDGA_Attack>())
				{
					if (!TagToUse.IsValid() || (LegacyLightAttackTag.IsValid() && TagToUse == LegacyLightAttackTag))
					{
						TagToUse = ResolveBasicAttackTag();
						UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: NORMALIZED KDGA_Attack subclass -> %s (fallback)"),
							*TagToUse.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Using KDGA_Attack subclass definition tag %s"), *TagToUse.ToString());
					}
				}
				else if (AbilityClass->IsChildOf<UKDGA_ComboAttack>())
				{
					if (!TagToUse.IsValid())
					{
						TagToUse = ResolveComboLightTag();
						UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: NORMALIZED UKDGA_ComboAttack -> %s (CODE-OWNED fallback)"),
							*TagToUse.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Using KDGA_ComboAttack definition tag %s"), *TagToUse.ToString());
					}
				}
			}

			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Granting ability %s with tag %s"),
				*GetNameSafe(AbilityClass),
				TagToUse.IsValid() ? *TagToUse.ToString() : TEXT("None"));

			AbilitySystemComp->GrantAbility(AbilityClass, TagToUse);
		}

		// Safety fallback: if the combo skill is not granted yet but the project-level
		// combo Blueprint exists, auto-grant it. This keeps hotbar testing code-owned
		// and avoids forcing a manual DefaultAbilities edit for the first playable slice.
		// Skip if the active build explicitly has no combo (ComboAbility is empty).
		const bool bBuildWantsNoCombo = ActiveBuildDefinition && !ActiveBuildDefinition->ComboAbility.IsValid();
		const bool bMasteryTestMode = FKDMasteryTestBootstrap::IsTestModeEnabled();
		const FGameplayTag ComboTag = ResolveComboLightTag();
		if (!bBuildWantsNoCombo && !bMasteryTestMode && ComboTag.IsValid() && !AbilitySystemComp->GetAbilityHandleByTag(ComboTag).IsValid())
		{
			if (TSubclassOf<UGameplayAbility> ComboAbilityClass = LoadDefaultComboAbilityClass())
			{
				UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Auto-granting fallback combo ability %s with tag %s"),
					*GetNameSafe(ComboAbilityClass),
					*ComboTag.ToString());
				AbilitySystemComp->GrantAbility(ComboAbilityClass, ComboTag);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: Combo tag is valid but fallback combo ability asset could not be loaded"));
			}
		}
		else if (bMasteryTestMode)
		{
			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Skipping legacy combo fallback while mastery test mode is active"));
		}

		// Safety fallback: if the basic attack is not granted yet, auto-grant the
		// branch-appropriate basic attack. This covers the case where the build
		// definition's BasicAttackAbility tag isn't in the resolver table.
		const FGameplayTag BasicTag = ResolveBasicAttackTag();
		if (BasicTag.IsValid() && !AbilitySystemComp->GetAbilityHandleByTag(BasicTag).IsValid())
		{
			// Try branch-specific basic attack Blueprints first, then shared shell
			TSubclassOf<UGameplayAbility> BasicAbilityClass = nullptr;
			if (MasteryIdentity.MasteryBranch == EKDMasteryBranch::Wizard)
			{
				BasicAbilityClass = FSoftClassPath(TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_BasicAttack.BP_GA_Wizard_BasicAttack_C")).TryLoadClass<UGameplayAbility>();
			}
			if (!BasicAbilityClass)
			{
				BasicAbilityClass = FSoftClassPath(TEXT("/Game/KingdawnCombat/Abilities/BP_GA_Attack.BP_GA_Attack_C")).TryLoadClass<UGameplayAbility>();
			}
			if (BasicAbilityClass)
			{
				UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Auto-granting fallback basic attack %s with tag %s"),
					*GetNameSafe(BasicAbilityClass), *BasicTag.ToString());
				AbilitySystemComp->GrantAbility(BasicAbilityClass, BasicTag);
			}
		}

		// Initialize stats component with ASC
		if (StatsComp)
		{
			StatsComp->InitializeWithAbilitySystem(AbilitySystemComp);
		}
	}

	bAbilitySystemInitialized = true;
}

EKDCombatClass AKDCombatCharacter::GetEntityCombatClass_Implementation() const
{
	return CombatClass;
}

EKDCombatBranch AKDCombatCharacter::GetEntityCombatBranch_Implementation() const
{
	return CombatBranch;
}

EKDWeaponType AKDCombatCharacter::GetEntityWeaponType_Implementation() const
{
	return EquippedWeapon;
}

FKDMasteryIdentity AKDCombatCharacter::GetEntityMasteryIdentity_Implementation() const
{
	return MasteryIdentity;
}

void AKDCombatCharacter::SetCombatClass(EKDCombatClass NewClass)
{
	CombatClass = NewClass;
	// Sync mastery identity from legacy fields
	MasteryIdentity.MasteryClass = ConvertToMasteryClass(CombatClass);
}

void AKDCombatCharacter::SetCombatBranch(EKDCombatBranch NewBranch)
{
	CombatBranch = NewBranch;
	// Sync mastery identity from legacy fields
	MasteryIdentity.MasteryBranch = ConvertToMasteryBranch(CombatBranch);
}

void AKDCombatCharacter::SetMasteryIdentity(FKDMasteryIdentity NewIdentity)
{
	MasteryIdentity = NewIdentity;
	// Sync legacy mirror fields from mastery identity
	CombatClass = ConvertToLegacyClass(MasteryIdentity.MasteryClass);
	CombatBranch = ConvertToLegacyBranch(MasteryIdentity.MasteryBranch);
}

void AKDCombatCharacter::SetEquippedWeapon(EKDWeaponType NewWeapon)
{
	if (EquippedWeapon == NewWeapon)
	{
		return;
	}

	EquippedWeapon = NewWeapon;
	UpdateWeaponMesh();
}

void AKDCombatCharacter::UpdateStatusDebugIndicator()
{
	if (!StatusDebugTextComp)
	{
		return;
	}

	if (IsPlayerControlled() || !AbilitySystemComp)
	{
		StatusDebugTextComp->SetHiddenInGame(true);
		StatusDebugTextComp->SetVisibility(false);
		StatusDebugTextComp->SetText(FText::GetEmpty());
		return;
	}

	TArray<FString> ActiveStatuses;
	if (AbilitySystemComp->HasMatchingGameplayTag(FKDGameplayTags::Status_Effect_Freeze))
	{
		ActiveStatuses.Add(TEXT("FREEZE"));
	}
	if (AbilitySystemComp->HasMatchingGameplayTag(FKDGameplayTags::Status_Effect_Slow))
	{
		ActiveStatuses.Add(TEXT("SLOW"));
	}
	if (AbilitySystemComp->HasMatchingGameplayTag(FKDGameplayTags::Status_Effect_Knockdown))
	{
		ActiveStatuses.Add(TEXT("KD"));
	}
	if (AbilitySystemComp->HasMatchingGameplayTag(FKDGameplayTags::Status_Effect_Bleed))
	{
		ActiveStatuses.Add(TEXT("BLEED"));
	}

	if (ActiveStatuses.Num() == 0)
	{
		StatusDebugTextComp->SetHiddenInGame(true);
		StatusDebugTextComp->SetVisibility(false);
		StatusDebugTextComp->SetText(FText::GetEmpty());
		return;
	}

	const FString Joined = FString::Join(ActiveStatuses, TEXT(" | "));
	StatusDebugTextComp->SetText(FText::FromString(Joined));
	StatusDebugTextComp->SetHiddenInGame(false);
	StatusDebugTextComp->SetVisibility(true);
}

void AKDCombatCharacter::UpdateWeaponMesh()
{
	if (!WeaponMeshComp || !MainHandWeaponMeshComp || !OffHandWeaponMeshComp)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter::UpdateWeaponMesh - Start (EquippedWeapon=%s, HasActiveBuild=%d)"),
		*UEnum::GetValueAsString(EquippedWeapon),
		ActiveBuildDefinition != nullptr);

	// Clear all weapon meshes first
	WeaponMeshComp->SetStaticMesh(nullptr);
	WeaponMeshComp->SetVisibility(false);
	MainHandWeaponMeshComp->SetSkeletalMesh(nullptr);
	MainHandWeaponMeshComp->SetVisibility(false);
	OffHandWeaponMeshComp->SetSkeletalMesh(nullptr);
	OffHandWeaponMeshComp->SetVisibility(false);

	USkeletalMeshComponent* CharMesh = GetMesh();
	if (!CharMesh)
	{
		return;
	}

	// Build-first: if ActiveBuildDefinition exists, apply weapon visuals from profiles
	if (ActiveBuildDefinition)
	{
		bool bPrimaryApplied = false;
		bool bOffhandApplied = false;

		// Primary weapon
		if (!ActiveBuildDefinition->PrimaryWeaponVisualProfile.IsNull())
		{
			if (const UKDWeaponVisualProfile* PrimaryProfile = ActiveBuildDefinition->PrimaryWeaponVisualProfile.LoadSynchronous())
			{
				bPrimaryApplied = ApplyWeaponVisualProfile(MainHandWeaponMeshComp, CharMesh, PrimaryProfile);
				if (bPrimaryApplied)
				{
					UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Applied primary weapon visual from build: %s"),
						*PrimaryProfile->GetName());
				}
			}
		}

		// Offhand weapon
		if (!ActiveBuildDefinition->OffhandWeaponVisualProfile.IsNull())
		{
			if (const UKDWeaponVisualProfile* OffhandProfile = ActiveBuildDefinition->OffhandWeaponVisualProfile.LoadSynchronous())
			{
				bOffhandApplied = ApplyWeaponVisualProfile(OffHandWeaponMeshComp, CharMesh, OffhandProfile);
				if (bOffhandApplied)
				{
					UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Applied offhand weapon visual from build: %s"),
						*OffhandProfile->GetName());
				}
			}
		}

		if (bPrimaryApplied || bOffhandApplied)
		{
			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Build-driven weapon visuals applied (primary=%d offhand=%d) - weapon mesh update complete"),
				bPrimaryApplied, bOffhandApplied);
			return;
		}

		// If build exists but no visuals resolved, fall through to legacy using build's PrimaryWeaponType
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: Build '%s' has no weapon visual profiles or profiles failed to load - falling back to legacy mesh logic with EquippedWeapon=%s"),
			*ActiveBuildDefinition->GetName(), *UEnum::GetValueAsString(EquippedWeapon));
	}

	// Legacy path: use the single weapon mesh component based on EquippedWeapon
	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Legacy weapon path - EquippedWeapon=%s"), *UEnum::GetValueAsString(EquippedWeapon));
	
	EKDWeaponFamily WeaponFamily = GetWeaponFamilyForType(EquippedWeapon);
	if (WeaponFamily == EKDWeaponFamily::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: EquippedWeapon=%s has no weapon family mapping - no weapon shown"),
			*UEnum::GetValueAsString(EquippedWeapon));
		return;
	}

	FString MeshPath;
	switch (EquippedWeapon)
	{
	case EKDWeaponType::Staff:
		MeshPath = TEXT("/Game/KingdawnCombat/Weapons/SM_wp_staff.SM_wp_staff");
		break;

	default:
		UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: No mesh path defined for weapon type %s"), *UEnum::GetValueAsString(EquippedWeapon));
		return;
	}

	if (MeshPath.IsEmpty())
	{
		return;
	}

	UStaticMesh* LoadedMesh = Cast<UStaticMesh>(FSoftObjectPath(MeshPath).TryLoad());
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: Failed to load weapon mesh from %s"), *MeshPath);
		return;
	}

	WeaponMeshComp->SetStaticMesh(LoadedMesh);
	WeaponMeshComp->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Legacy path loaded weapon mesh '%s' on WeaponMeshComp"), *LoadedMesh->GetName());

	if (const FKDWeaponAttachProfile* Profile = WeaponProfiles.Find(WeaponFamily))
	{
		AttachWeaponByProfile(WeaponMeshComp, CharMesh, *Profile);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: No attachment profile found for weapon family %s"), *UEnum::GetValueAsString(WeaponFamily));
	}
	
	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter::UpdateWeaponMesh - Complete (legacy path)"));
}

EKDWeaponFamily AKDCombatCharacter::GetWeaponFamilyForType(EKDWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case EKDWeaponType::Staff:
		return EKDWeaponFamily::Staff;
	case EKDWeaponType::OneHandedSword:
		return EKDWeaponFamily::OneHandedSword;
	case EKDWeaponType::Shield:
		return EKDWeaponFamily::Shield;
	case EKDWeaponType::TwoHandedSword:
		return EKDWeaponFamily::TwoHandedSword;
	case EKDWeaponType::Bow:
		return EKDWeaponFamily::Bow;
	case EKDWeaponType::Dagger:
		return EKDWeaponFamily::Dagger;
	case EKDWeaponType::Axe:
		return EKDWeaponFamily::Axe;
	case EKDWeaponType::Hammer:
		return EKDWeaponFamily::Hammer;
	case EKDWeaponType::Wand:
		return EKDWeaponFamily::Wand;
	default:
		return EKDWeaponFamily::None;
	}
}

FKDWeaponAttachProfile AKDCombatCharacter::GetWeaponAttachmentProfile(EKDWeaponFamily Family) const
{
	const FKDWeaponAttachProfile* Found = WeaponProfiles.Find(Family);
	return Found ? *Found : FKDWeaponAttachProfile();
}

void AKDCombatCharacter::SetWeaponAttachmentProfile(EKDWeaponFamily Family, const FKDWeaponAttachProfile& Profile)
{
	WeaponProfiles.Add(Family, Profile);
	// Re-equip current weapon to apply the new profile
	UpdateWeaponMesh();
}

namespace
{
	const USkeletalMeshSocket* FindReferenceSocketWithFallbacks(UObject* ReferenceObject, FName PrimarySocket)
	{
		auto TryFind = [ReferenceObject](FName SocketName) -> const USkeletalMeshSocket*
		{
			if (SocketName.IsNone())
			{
				return nullptr;
			}

			if (const USkeletalMesh* RefMesh = Cast<USkeletalMesh>(ReferenceObject))
			{
				return RefMesh->FindSocket(SocketName);
			}
			if (const USkeleton* RefSkeleton = Cast<USkeleton>(ReferenceObject))
			{
				return RefSkeleton->FindSocket(SocketName);
			}
			return nullptr;
		};

		if (const USkeletalMeshSocket* Found = TryFind(PrimarySocket))
		{
			return Found;
		}

		TArray<FName> Alternates;
		if (PrimarySocket == FName(TEXT("weapon_rSocket")))
		{
			// Support multiple naming conventions for right-hand weapon socket
			Alternates = { FName(TEXT("weapon_rsocket")), FName(TEXT("hand_rSocket")), FName(TEXT("hand_rsocket")), FName(TEXT("blade_rSocket")) };
		}
		else if (PrimarySocket == FName(TEXT("shield_lSocket")))
		{
			// Support multiple naming conventions for left-hand shield socket
			Alternates = { FName(TEXT("shield_Lsocket")), FName(TEXT("Shield_lSocket")), FName(TEXT("Shield_Lsocket")), FName(TEXT("Shield_1")), FName(TEXT("shield_l")), FName(TEXT("hand_lSocket")), FName(TEXT("hand_Lsocket")) };
		}
		else if (PrimarySocket == FName(TEXT("hand_rSocket")))
		{
			Alternates = { FName(TEXT("weapon_rSocket")), FName(TEXT("weapon_rsocket")), FName(TEXT("hand_rsocket")) };
		}
		else if (PrimarySocket == FName(TEXT("weapon_rsocket")))
		{
			// If primary is lowercase variant, try uppercase variants
			Alternates = { FName(TEXT("weapon_rSocket")), FName(TEXT("hand_rSocket")), FName(TEXT("hand_rsocket")) };
		}
		else if (PrimarySocket == FName(TEXT("shield_Lsocket")))
		{
			// If primary is uppercase L variant, try other variants
			Alternates = { FName(TEXT("shield_lSocket")), FName(TEXT("Shield_lSocket")), FName(TEXT("Shield_Lsocket")), FName(TEXT("shield_l")) };
		}

		for (const FName Alt : Alternates)
		{
			if (const USkeletalMeshSocket* Found = TryFind(Alt))
			{
				return Found;
			}
		}

		return nullptr;
	}
}
void AKDCombatCharacter::AttachWeaponByProfile(USceneComponent* WeaponComp, USkeletalMeshComponent* CharMesh, const FKDWeaponAttachProfile& Profile)
{
	if (!WeaponComp || !CharMesh)
	{
		return;
	}

	// Resolve attachment target: prefer authored socket, fall back to raw bone
	FName AttachTarget = Profile.SocketName;
	bool bSocketExistsOnRuntime = CharMesh->DoesSocketExist(Profile.SocketName);

	if (!bSocketExistsOnRuntime && !Profile.BoneName.IsNone())
	{
		// Socket not on runtime mesh ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â use bone as attachment point
		AttachTarget = Profile.BoneName;
		UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Socket %s not found on runtime mesh, falling back to bone %s"),
			*Profile.SocketName.ToString(), *Profile.BoneName.ToString());
	}

	if (AttachTarget.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: No valid attach target for weapon (socket=%s bone=%s)"),
			*Profile.SocketName.ToString(), *Profile.BoneName.ToString());
		return;
	}

	// Attach to the resolved socket/bone
	WeaponComp->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, AttachTarget);

	// If the runtime mesh has the authored socket, the socket transform is already correct.
	// Just apply the family-level offsets on top.
	if (bSocketExistsOnRuntime)
	{
		WeaponComp->SetRelativeLocation(Profile.RelativeOffset);
		WeaponComp->SetRelativeRotation(Profile.RelativeRotation);
		WeaponComp->SetRelativeScale3D(Profile.RelativeScale);
	}
	else
	{
		// Socket not on runtime mesh ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€Â¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â try reference mesh fallback first.
		// This preserves the Witch reference-socket approach for the Wizard staff.
		FVector FinalLocation = Profile.RelativeOffset;
		FRotator FinalRotation = Profile.RelativeRotation;
		FVector FinalScale = Profile.RelativeScale;

		if (!Profile.ReferenceMeshPath.IsNull() && !Profile.ReferenceSocketName.IsNone())
		{
			if (UObject* ReferenceObject = Profile.ReferenceMeshPath.TryLoad())
			{
				const USkeletalMeshSocket* RefSocket = FindReferenceSocketWithFallbacks(ReferenceObject, Profile.ReferenceSocketName);

				if (RefSocket)
				{
					// Use reference socket transform as base, then add profile offsets
					FinalLocation = RefSocket->RelativeLocation + Profile.RelativeOffset;
					FinalRotation = RefSocket->RelativeRotation + Profile.RelativeRotation;
					FinalScale = Profile.RelativeScale;
					UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Applied reference socket fallback from %s:%s"),
						*Profile.ReferenceMeshPath.ToString(), *Profile.ReferenceSocketName.ToString());
				}
			}
		}

		WeaponComp->SetRelativeLocation(FinalLocation);
		WeaponComp->SetRelativeRotation(FinalRotation);
		WeaponComp->SetRelativeScale3D(FinalScale);
	}

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Attached weapon to %s (socket existed=%d) loc=(%s) rot=(%s)"),
		*AttachTarget.ToString(), bSocketExistsOnRuntime,
		*WeaponComp->GetRelativeLocation().ToString(),
		*WeaponComp->GetRelativeRotation().ToString());
}

bool AKDCombatCharacter::TryApplyReferenceSocketTransform(USceneComponent* WeaponComp, const FKDWeaponAttachProfile& Profile)
{
	if (!WeaponComp || Profile.ReferenceMeshPath.IsNull() || Profile.ReferenceSocketName.IsNone())
	{
		return false;
	}

	if (UObject* ReferenceObject = Profile.ReferenceMeshPath.TryLoad())
	{
		const USkeletalMeshSocket* ReferenceSocket = FindReferenceSocketWithFallbacks(ReferenceObject, Profile.ReferenceSocketName);

		if (ReferenceSocket)
		{
			WeaponComp->SetRelativeLocation(ReferenceSocket->RelativeLocation);
			WeaponComp->SetRelativeRotation(ReferenceSocket->RelativeRotation);
			WeaponComp->SetRelativeScale3D(ReferenceSocket->RelativeScale);
			UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Applied reference socket transform from %s:%s"),
				*Profile.ReferenceMeshPath.ToString(),
				*Profile.ReferenceSocketName.ToString());
			return true;
		}
	}

	return false;
}

// --- Build System ---

bool AKDCombatCharacter::ApplyBuildDefinition(UKDBuildDefinition* BuildDef)
{
	if (!BuildDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter::ApplyBuildDefinition - null BuildDef, clearing active build"));
		ActiveBuildDefinition = nullptr;
		return false;
	}

	// Validate that the build has meaningful data.
	// Check mastery identity first (primary); fall back to legacy fields for existing assets.
	const bool bHasMasteryIdentity = BuildDef->MasteryIdentity.IsValid();
	const bool bHasLegacyIdentity = BuildDef->CombatClass != EKDCombatClass::None || BuildDef->CombatBranch != EKDCombatBranch::None;
	if (!bHasMasteryIdentity && !bHasLegacyIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter::ApplyBuildDefinition - build '%s' has no class or branch set, ignoring"),
			*BuildDef->GetName());
		return false;
	}

	// If the build has mastery identity set, ensure legacy fields are synced.
	// If only legacy fields are set (old data asset), sync mastery from legacy.
	if (bHasMasteryIdentity && !bHasLegacyIdentity)
	{
		BuildDef->SyncLegacyFromMastery();
	}
	else if (!bHasMasteryIdentity && bHasLegacyIdentity)
	{
		BuildDef->SyncMasteryFromLegacy();
	}

	// Cache the active build for later use by ability/hotbar/visual systems
	ActiveBuildDefinition = BuildDef;

	// Apply mastery identity from the build (primary authority)
	MasteryIdentity = BuildDef->MasteryIdentity;

	// Sync legacy mirror fields from mastery identity
	CombatClass = ConvertToLegacyClass(MasteryIdentity.MasteryClass);
	CombatBranch = ConvertToLegacyBranch(MasteryIdentity.MasteryBranch);

	// Apply weapon state (independent from mastery identity)
	EquippedWeapon = BuildDef->PrimaryWeaponType;

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter::ApplyBuildDefinition - applied build '%s' (Class=%s, Branch=%s, PrimaryWeapon=%s, Offhand=%s)"),
		*BuildDef->GetName(),
		*UEnum::GetValueAsString(CombatClass),
		*UEnum::GetValueAsString(CombatBranch),
		*UEnum::GetValueAsString(EquippedWeapon),
		*UEnum::GetValueAsString(BuildDef->OffhandWeaponType));

	// Log ability tags for debugging (actual ability granting deferred to Phase 3+)
	if (BuildDef->BasicAttackAbility.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter::ApplyBuildDefinition - BasicAttack: %s"), *BuildDef->BasicAttackAbility.ToString());
	}
	if (BuildDef->ComboAbility.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter::ApplyBuildDefinition - ComboAbility: %s"), *BuildDef->ComboAbility.ToString());
	}
	if (BuildDef->DefaultAbilities.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter::ApplyBuildDefinition - %d default abilities tagged"), BuildDef->DefaultAbilities.Num());
	}

	return true;
}

bool AKDCombatCharacter::ApplyWeaponVisualProfile(USkeletalMeshComponent* WeaponComp, USkeletalMeshComponent* CharMesh, const UKDWeaponVisualProfile* Profile)
{
	if (!WeaponComp || !CharMesh || !Profile)
	{
		return false;
	}

	// Load the mesh asset from the profile
	if (Profile->MeshAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: WeaponVisualProfile %s has no MeshAsset"), *Profile->GetName());
		return false;
	}

	USkeletalMesh* LoadedMesh = Profile->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCombatCharacter: Failed to load mesh asset from profile %s"), *Profile->GetName());
		return false;
	}

	// Set the mesh
	WeaponComp->SetSkeletalMesh(LoadedMesh);

	// Build an attachment profile from the visual profile data
	FKDWeaponAttachProfile AttachProfile;
	AttachProfile.BoneName = Profile->AttachBoneName;
	AttachProfile.SocketName = Profile->SocketName;
	AttachProfile.RelativeOffset = Profile->RelativeOffset;
	AttachProfile.RelativeRotation = Profile->RelativeRotation;
	AttachProfile.RelativeScale = Profile->RelativeScale;

	// Handle reference-socket fallback
	if (Profile->bUseReferenceSocketFallback && !Profile->ReferenceAsset.IsNull())
	{
		AttachProfile.ReferenceMeshPath = FSoftObjectPath(Profile->ReferenceAsset.ToSoftObjectPath());
		AttachProfile.ReferenceSocketName = Profile->ReferenceSocketName;
	}

	// Attach using the existing profile-driven attachment logic
	AttachWeaponByProfile(WeaponComp, CharMesh, AttachProfile);

	// Make visible
	WeaponComp->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("KDCombatCharacter: Applied weapon visual profile %s (family=%s mesh=%s)"),
		*Profile->GetName(),
		*UEnum::GetValueAsString(Profile->WeaponFamily),
		*LoadedMesh->GetName());

	return true;
}














