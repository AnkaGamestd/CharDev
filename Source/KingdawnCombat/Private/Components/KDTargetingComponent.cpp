// Copyright Kingdawn. All Rights Reserved.

#include "Components/KDTargetingComponent.h"
#include "Components/KDDamageComponent.h"
#include "Interfaces/KDEntityInterface.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UKDTargetingComponent::UKDTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	MaxTargetDistance = 3000.f;
	MaxCycleDistance = 2000.f;
	ValidationInterval = 0.5f;
	LineOfSightChannel = ECC_Visibility;
	TimeSinceValidation = 0.f;

	// Debug visualization
	bShowDebugTargetIndicator = true; // Enable by default for testing
	DebugTargetColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
	DebugTargetRingRadius = 100.f;
}

void UKDTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UKDTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Periodic validation
	if (ValidationInterval > 0.f)
	{
		TimeSinceValidation += DeltaTime;
		if (TimeSinceValidation >= ValidationInterval)
		{
			ValidateCurrentTarget();
			TimeSinceValidation = 0.f;
		}
	}
	else
	{
		ValidateCurrentTarget(); // Every frame if interval is 0
	}

	// Debug visualization: draw ring around current target
	if (bShowDebugTargetIndicator && CurrentTarget.IsValid())
	{
		AActor* Target = CurrentTarget.Get();
		const FVector TargetLocation = Target->GetActorLocation();
		const FVector RingCenter = TargetLocation + FVector(0.f, 0.f, 10.f); // Slightly above ground

		DrawDebugCircle(
			GetWorld(),
			RingCenter,
			DebugTargetRingRadius,
			32, // Segments
			DebugTargetColor.ToFColor(true),
			false, // Persistent
			-1.f, // Lifetime (one frame)
			0, // Depth priority
			3.f // Thickness
		);
	}
}

// --- Target Selection ---

void UKDTargetingComponent::SetTarget(AActor* NewTarget)
{
	if (NewTarget == CurrentTarget.Get())
	{
		return; // Already selected
	}

	AActor* OldTarget = CurrentTarget.Get();

	if (NewTarget && !IsValidHostileTarget(NewTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("KDTargetingComponent: Cannot target %s - not a valid hostile"), *NewTarget->GetName());
		return;
	}

	CurrentTarget = NewTarget;
	OnTargetChanged.Broadcast(OldTarget, NewTarget);
}

void UKDTargetingComponent::ClearTarget()
{
	if (CurrentTarget.IsValid())
	{
		AActor* OldTarget = CurrentTarget.Get();
		CurrentTarget = nullptr;
		OnTargetChanged.Broadcast(OldTarget, nullptr);
	}
}

AActor* UKDTargetingComponent::GetCurrentTarget() const
{
	return CurrentTarget.Get();
}

AActor* UKDTargetingComponent::GetValidTarget() const
{
	if (IsCurrentTargetValid())
	{
		return CurrentTarget.Get();
	}
	return nullptr;
}

// --- Validation ---

bool UKDTargetingComponent::IsValidHostileTarget(AActor* Target) const
{
	if (!Target || !GetOwner())
	{
		return false;
	}

	// Must be alive
	if (!IsActorAlive(Target))
	{
		return false;
	}

	// Must implement IKDEntityInterface for team checking
	if (!Target->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass()))
	{
		return false; // Cannot target actors without entity interface
	}

	// Must be hostile (different team)
	const bool bOwnerImplements = GetOwner()->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass());
	if (bOwnerImplements)
	{
		const FGameplayTag OwnerTeam = IKDEntityInterface::Execute_GetEntityCombatTeam(GetOwner());
		const FGameplayTag TargetTeam = IKDEntityInterface::Execute_GetEntityCombatTeam(Target);

		// Same team = not hostile
		if (OwnerTeam.IsValid() && TargetTeam.IsValid() && OwnerTeam == TargetTeam)
		{
			return false;
		}
	}

	return true;
}

bool UKDTargetingComponent::IsCurrentTargetValid() const
{
	if (!CurrentTarget.IsValid())
	{
		return false;
	}

	AActor* Target = CurrentTarget.Get();

	// Check if still a valid hostile target
	if (!IsValidHostileTarget(Target))
	{
		return false;
	}

	// Check distance
	const float Distance = GetTargetDistance();
	if (Distance > MaxTargetDistance)
	{
		return false;
	}

	return true;
}

bool UKDTargetingComponent::IsTargetInRange(float Range) const
{
	const float Distance = GetTargetDistance();
	return Distance >= 0.f && Distance <= Range;
}

float UKDTargetingComponent::GetTargetDistance() const
{
	if (!CurrentTarget.IsValid() || !GetOwner())
	{
		return -1.f;
	}

	return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

// --- Target Cycling ---

void UKDTargetingComponent::CycleTarget(bool bForward)
{
	TArray<AActor*> HostileTargets = FindHostileTargetsInRange(MaxCycleDistance);

	if (HostileTargets.Num() == 0)
	{
		return; // No targets available
	}

	// If no current target, select nearest
	if (!CurrentTarget.IsValid())
	{
		SetTarget(HostileTargets[0]);
		return;
	}

	// Find current target in list
	int32 CurrentIndex = HostileTargets.IndexOfByKey(CurrentTarget.Get());
	if (CurrentIndex == INDEX_NONE)
	{
		// Current target not in list (maybe died or out of range), select nearest
		SetTarget(HostileTargets[0]);
		return;
	}

	// Cycle to next/previous
	int32 NextIndex = bForward ? (CurrentIndex + 1) % HostileTargets.Num() : (CurrentIndex - 1 + HostileTargets.Num()) % HostileTargets.Num();
	SetTarget(HostileTargets[NextIndex]);
}

void UKDTargetingComponent::SelectNearestTarget()
{
	TArray<AActor*> HostileTargets = FindHostileTargetsInRange(MaxCycleDistance);

	if (HostileTargets.Num() > 0)
	{
		SetTarget(HostileTargets[0]); // Already sorted by distance
	}
}

// --- Internal ---

void UKDTargetingComponent::ValidateCurrentTarget()
{
	if (!IsCurrentTargetValid())
	{
		ClearTarget();
	}
}

TArray<AActor*> UKDTargetingComponent::FindHostileTargetsInRange(float Range) const
{
	TArray<AActor*> Result;

	if (!GetOwner() || !GetWorld())
	{
		return Result;
	}

	// Get all actors of class AActor (or use a more specific class if available)
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	// Filter and sort by distance
	struct FTargetDistance
	{
		AActor* Actor;
		float Distance;
	};
	TArray<FTargetDistance> ValidTargets;

	for (AActor* Actor : AllActors)
	{
		if (Actor == GetOwner())
		{
			continue; // Skip self
		}

		const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
		if (Distance > Range)
		{
			continue; // Out of range
		}

		if (IsValidHostileTarget(Actor))
		{
			ValidTargets.Add({ Actor, Distance });
		}
	}

	// Sort by distance
	ValidTargets.Sort([](const FTargetDistance& A, const FTargetDistance& B) {
		return A.Distance < B.Distance;
	});

	// Extract actors
	for (const FTargetDistance& Target : ValidTargets)
	{
		Result.Add(Target.Actor);
	}

	return Result;
}

bool UKDTargetingComponent::IsActorAlive(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// Check if actor has a damage component and is alive
	UKDDamageComponent* DamageComp = Actor->FindComponentByClass<UKDDamageComponent>();
	if (DamageComp)
	{
		return DamageComp->GetIsAlive();
	}

	// Fallback: assume alive if no damage component
	return true;
}
