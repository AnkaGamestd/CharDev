// Copyright Kingdawn. All Rights Reserved.

#include "Components/KDClickInputHandler.h"
#include "Characters/KDCombatCharacter.h"
#include "Components/KDAbilitySystemComponent.h"
#include "Components/KDTargetingComponent.h"
#include "Core/KDGameplayTags.h"
#include "Interfaces/KDEntityInterface.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "DrawDebugHelpers.h"

UKDClickInputHandler::UKDClickInputHandler()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Only tick when pending double-click

	DoubleClickThreshold = 0.2f;
	ClickTraceChannel = ECC_Pawn; // Use Pawn channel so we can click on characters
	MaxClickTraceDistance = 100000.f;

	bPendingDoubleClick = false;
	LastClickTime = -999.f;
	LastClickHitLocation = FVector::ZeroVector;
}

void UKDClickInputHandler::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner controller
	OwnerController = Cast<APlayerController>(GetOwner());
}

void UKDClickInputHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check if pending single-click has expired (double-click window closed)
	if (bPendingDoubleClick)
	{
		const float TimeSinceLastClick = GetWorld()->GetTimeSeconds() - LastClickTime;
		if (TimeSinceLastClick > DoubleClickThreshold)
		{
			ProcessSingleClick();
			bPendingDoubleClick = false;
			SetComponentTickEnabled(false);
		}
	}
	else
	{
		// No reason to tick
		SetComponentTickEnabled(false);
	}
}

// --- Input Callbacks ---

void UKDClickInputHandler::OnLeftClickPressed()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeSinceLastClick = CurrentTime - LastClickTime;

	// Perform raycast to see what we clicked on
	FVector HitLocation;
	AActor* ClickedActor = PerformClickRaycast(HitLocation);

	// BUG FIX: Check if this is a double-click on the SAME actor
	if (bPendingDoubleClick && TimeSinceLastClick <= DoubleClickThreshold && ClickedActor == LastClickedActor.Get())
	{
		// Double-click detected
		ProcessDoubleClick();
		bPendingDoubleClick = false;
		SetComponentTickEnabled(false);
	}
	else
	{
		// First click — start pending double-click window
		bPendingDoubleClick = true;
		LastClickTime = CurrentTime;
		LastClickedActor = ClickedActor;
		LastClickHitLocation = HitLocation; // Cache hit location

		// Enable tick to check for double-click timeout
		SetComponentTickEnabled(true);
	}
}

void UKDClickInputHandler::OnLeftClickReleased()
{
	// No action needed on release for Silkroad controls
}

// --- Single Click ---

void UKDClickInputHandler::ProcessSingleClick()
{
	AActor* ClickedActor = LastClickedActor.Get();

	// BUG FIX: Use cached hit location instead of re-raycasting
	if (ClickedActor && IsValidHostileForCharacter(ClickedActor))
	{
		// Valid hostile enemy — target only
		HandleClickOnEnemy(ClickedActor, false);
	}
	else
	{
		// Ground click or non-hostile actor — move
		HandleClickOnGround(LastClickHitLocation);
	}
}

// --- Double Click ---

void UKDClickInputHandler::ProcessDoubleClick()
{
	AActor* ClickedActor = LastClickedActor.Get();

	// BUG FIX: Use cached hit location instead of re-raycasting
	if (ClickedActor && IsValidHostileForCharacter(ClickedActor))
	{
		// Valid hostile enemy — target + attack
		HandleClickOnEnemy(ClickedActor, true);
	}
	else
	{
		// Ground click or non-hostile actor — move (no special double-click behavior for ground)
		HandleClickOnGround(LastClickHitLocation);
	}
}

// --- Raycast ---

AActor* UKDClickInputHandler::PerformClickRaycast(FVector& OutHitLocation)
{
	if (!OwnerController)
	{
		OutHitLocation = FVector::ZeroVector;
		return nullptr;
	}

	// Multi-channel raycast strategy:
	// 1. Try Pawn channel first (for clicking on characters via their CapsuleComponent)
	// 2. Try Visibility channel (for clicking on meshes and other visible objects)
	// 3. Try WorldDynamic (for dynamic actors)
	// 4. Fall back to WorldStatic (for floor/ground)
	
	FHitResult HitResult;
	
	// Attempt 1: Pawn channel - hits CapsuleComponents of Pawns
	if (OwnerController->GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_Pawn), false, HitResult))
	{
		OutHitLocation = HitResult.ImpactPoint;
		if (AActor* HitActor = HitResult.GetActor())
		{
			return HitActor;
		}
	}

	// Attempt 2: Visibility channel - hits meshes and other visible objects
	if (OwnerController->GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_Visibility), false, HitResult))
	{
		OutHitLocation = HitResult.ImpactPoint;
		if (AActor* HitActor = HitResult.GetActor())
		{
			return HitActor;
		}
	}

	// Attempt 3: WorldDynamic - for dynamic actors
	if (OwnerController->GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_WorldDynamic), false, HitResult))
	{
		OutHitLocation = HitResult.ImpactPoint;
		if (AActor* HitActor = HitResult.GetActor())
		{
			return HitActor;
		}
	}

	// Attempt 4: WorldStatic - floor and static geometry
	if (OwnerController->GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, HitResult))
	{
		OutHitLocation = HitResult.ImpactPoint;
		if (AActor* HitActor = HitResult.GetActor())
		{
			return HitActor;
		}
	}

	// Nothing hit - use deprojected cursor location
	FVector WorldLocation, WorldDirection;
	if (OwnerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		OutHitLocation = WorldLocation + (WorldDirection * MaxClickTraceDistance);
	}
	else
	{
		OutHitLocation = FVector::ZeroVector;
	}
	return nullptr;
}

// --- Enemy Click ---

void UKDClickInputHandler::HandleClickOnEnemy(AActor* Enemy, bool bIsDoubleClick)
{
	AKDCombatCharacter* Character = GetPossessedCharacter();
	if (!Character)
	{
		return;
	}

	// Always set target on enemy click
	Character->SetCombatTarget(Enemy);

	// If basic attack is already active, subsequent enemy click continues basic attack.
	// Note: We only check for basic attack tag here, NOT combo skill tag.
	// Combo skills are triggered via hotbar keys, not mouse clicks.
	if (UKDAbilitySystemComponent* KDASC = Character->GetKDAbilitySystemComponent())
	{
		if (KDASC->IsPerformingAbility() && KDASC->IsInAbilityState(FKDGameplayTags::Combat_Action_Attack_Basic))
		{
			Character->TryAttack();
			return;
		}
	}

	if (bIsDoubleClick)
	{
		// Double click: queue attack (auto-approach if needed)
		// BUG FIX: QueueAttackOnApproach will re-set target, but that's OK (idempotent)
		Character->QueueAttackOnApproach(Enemy);
	}
}

// --- Ground Click ---

void UKDClickInputHandler::HandleClickOnGround(const FVector& GroundLocation)
{
	AKDCombatCharacter* Character = GetPossessedCharacter();
	if (!Character)
	{
		return;
	}

	// Cancel any pending attack approach
	Character->CancelQueuedAttack();

	// FIX: Project the click location to the character's ground plane.
	// The raycast may hit geometry below the floor (Z=-80), which causes
	// SimpleMoveToLocation to fail because the destination is underground.
	// Solution: Use the click's X,Y but keep Z at the character's Z level.
	FVector CorrectedLocation = GroundLocation;
	
	// Use a line trace to find the actual floor Z at the clicked XY position
	FHitResult FloorHit;
	FVector TraceStart = FVector(GroundLocation.X, GroundLocation.Y, Character->GetActorLocation().Z + 500.f);
	FVector TraceEnd = FVector(GroundLocation.X, GroundLocation.Y, Character->GetActorLocation().Z - 500.f);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	if (GetWorld()->LineTraceSingleByChannel(FloorHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		CorrectedLocation = FloorHit.ImpactPoint;
	}
	else
	{
		// No floor found at click location — use character's Z
		CorrectedLocation.Z = Character->GetActorLocation().Z;
	}

	UE_LOG(LogTemp, Log, TEXT("KDClickInput: Ground click raw=(%.1f, %.1f, %.1f) → corrected=(%.1f, %.1f, %.1f)"),
		GroundLocation.X, GroundLocation.Y, GroundLocation.Z,
		CorrectedLocation.X, CorrectedLocation.Y, CorrectedLocation.Z);

	// Spawn floor marker at corrected location (on the actual floor)
	Character->SpawnFloorMarker(CorrectedLocation);

	// Set movement target for rotation (character will face this point)
	Character->SetMovementTarget(CorrectedLocation);

	// Move to the corrected location
	if (OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDClickInput: Moving to (%.1f, %.1f, %.1f), CharPos=(%.1f, %.1f, %.1f)"),
			CorrectedLocation.X, CorrectedLocation.Y, CorrectedLocation.Z,
			Character->GetActorLocation().X, Character->GetActorLocation().Y, Character->GetActorLocation().Z);
		
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(OwnerController, CorrectedLocation);
	}
}

// --- Helpers ---

AKDCombatCharacter* UKDClickInputHandler::GetPossessedCharacter()
{
	// BUG FIX: Auto-invalidate cache if pawn changed
	if (OwnerController && (!PossessedCharacter.IsValid() || PossessedCharacter.Get() != OwnerController->GetPawn()))
	{
		PossessedCharacter = Cast<AKDCombatCharacter>(OwnerController->GetPawn());
	}
	return PossessedCharacter.Get();
}

bool UKDClickInputHandler::IsValidHostileForCharacter(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// Need to cast away const to call GetPossessedCharacter (which caches)
	UKDClickInputHandler* MutableThis = const_cast<UKDClickInputHandler*>(this);
	AKDCombatCharacter* Character = MutableThis->GetPossessedCharacter();
	
	if (!Character || !Character->GetTargetingComponent())
	{
		return false;
	}

	return Character->GetTargetingComponent()->IsValidHostileTarget(Actor);
}
